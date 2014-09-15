#include "GPUrayscene.h"
#include "view3d.h"
#include <QList>
#include "global.h"
#include "camera.h"
#include "resource_loader.h"
extern "C" {
    void glBindBuffer (GLenum target, GLuint buffer);
    void  glGenBuffers (GLsizei n, GLuint *buffers);
    void  glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    void glBufferSubData( GLenum target,GLintptr offset, GLsizeiptr size,const GLvoid * data);
    void glGetBufferParameteriv( GLenum target, GLenum value, GLint * data);
    void glDeleteBuffers( GLsizei n, const GLuint* buffers );
}

GPURayScene::GPURayScene()
{
    m_cl = NULL;
    m_cmGlobal = NULL;
    m_cmLight = NULL;
    m_cmObject = NULL;
}

GPURayScene::GPURayScene( CLPack* cl,
                          OrbitCamera* camera,
                          Scene* scene,
                          GLuint screenTexHandle,
                          GLuint screenPbo,
                          GLuint screenWidth,
                          GLuint screenHeight
                          )
{
    pushSceneData( scene );
    m_cl = cl;
    m_camera = camera;

    m_screenTex = screenTexHandle;
    m_screenPbo = screenPbo;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_cmGlobal = NULL;
    m_cmLight = NULL;
    m_cmObject = NULL;

    initCLBuffers();
    setKernelArgs();
    syncGlobalSettings();
}

GPURayScene::~GPURayScene()
{
    if( m_cmGlobal )
        clReleaseMemObject( m_cmGlobal );

    if( m_cmLight )
        clReleaseMemObject( m_cmLight );

    if( m_cmObject )
        clReleaseMemObject( m_cmObject );

    for( int i = 0; i < 4; i++ )
    {
        if( m_textures[i] )
            clReleaseMemObject( m_textures[i] );
    }

}

void GPURayScene::render()
{
    cl_int ciErrNum;
    // Sync gl calls
    glFinish();

    Vector4 eyePos = m_camera->getEyePos();
    Matrix4x4 invMat = m_camera->getInvViewTransMatrix();

    cl_float16 clInvMat = copySceneMatrix( invMat );
    cl_float4 clEyePos = copySceneVector4( eyePos );
     ciErrNum = clSetKernelArg(m_cl->m_kernelRay, 9, sizeof(cl_float4), (void*)&clEyePos);
     ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 10, sizeof(cl_float16), (void*)&clInvMat);

      if( ciErrNum != CL_SUCCESS )
      {
          cerr<<"Set kernel failed in line:"<<__LINE__<<", File:"<<__FILE__<<endl;
          return;
      }

      // before using GL object we need to acquire
    ciErrNum =  clEnqueueAcquireGLObjects( m_cl->m_queue, 1, &m_cl->m_cmPbo, 0, NULL, NULL );
   if( ciErrNum != CL_SUCCESS )
   {
       cerr<<"Acquire GL objects failed in line:"<<__LINE__<<", File:"<<__FILE__<<endl;
       return ;
   }

   // Aquire textures
   for( int i = 0; i < MIN(m_textureHandles.size(),4); i++ )
   {
       clEnqueueAcquireGLObjects( m_cl->m_queue, 1, &m_textures[i], 0,NULL,NULL );
   }
   // Launch the kernel
    ciErrNum  = clEnqueueNDRangeKernel( m_cl->m_queue,
                                        m_cl->m_kernelRay,
                                        2,
                                        NULL,
                                        m_cl->m_globalWorkSize,
                                        m_cl->m_localWorkSize,
                                        0,
                                        NULL,
                                        NULL
                                        );

    if( ciErrNum != CL_SUCCESS )
    {
        cerr<<"Launch kernel failed in line:"<<__LINE__<<", File:"<<__FILE__<<endl;
        return ;
    }

     clFinish( m_cl->m_queue );
    // Release buffer then GL can use
     clEnqueueReleaseGLObjects( m_cl->m_queue, 1, &m_cl->m_cmPbo, 0, NULL, NULL );
     // Aquire textures
     for( int i = 0; i < MIN(m_textureHandles.size(),4); i++ )
     {
         clEnqueueReleaseGLObjects( m_cl->m_queue, 1, &m_textures[i], 0,NULL,NULL );
     }
     // Sync
    clFinish( m_cl->m_queue );
    QVector<LightDataHost> test;
    test.resize( m_lightData.size() );

    // Now read back the buffer to texture
    updateScreenTexFromPBO();
    displayScreenTex();
}

/**
    Review passed
  **/
// Push the scene's data in vector form
void GPURayScene::pushSceneData( Scene* scene )
{
    const CS123SceneGlobalData global = scene->getGlobal();
    m_global.s0 = global.ka;
    m_global.s1 = global.kd;
    m_global.s2 = global.ks;
    m_global.s3 = global.kt;

    const QList<CS123SceneLightData> lights = scene->getLight();

    m_lightData.resize( lights.size() );
    for( int i = 0; i < lights.size(); i++ )
    {
        CS123SceneLightData curLight = lights[i];
        LightDataHost& newlight = m_lightData[i];
       newlight.id = curLight.id;
       newlight.type = (cl_int)curLight.type;
       newlight.color = copySceneColor( curLight.color );
       newlight.function = copySceneVector3(curLight.function);

       newlight.pos = copySceneVector4(curLight.pos);
       newlight.dir = copySceneVector4(curLight.dir);

       newlight.radius = curLight.radius;
       newlight.penumbra = curLight.penumbra;
       newlight.angle = curLight.angle;
       newlight.width = curLight.width;
       newlight.height = curLight.height;
    }
    assert( m_lightData.size() == lights.size() );

    const QList<SceneObject> objects = scene->getObjects();
    m_objects.resize( objects.size() );
    for( int i = 0; i < objects.size(); i++ )
    {
        SceneObject curObj = objects[i];

        ObjectDataHost& newObj = m_objects[i];

        newObj.transform = copySceneMatrix(curObj.m_transform);
        newObj.type = (cl_int)curObj.m_primitive.type;
        newObj.texHandle = (cl_int)curObj.m_texture.m_textureHandle;
        newObj.texMapID = (cl_int)curObj.m_texture.m_mapIndex;
        newObj.blend = curObj.m_primitive.material.blend;
        newObj.repeat.s0 = curObj.m_primitive.material.textureMap->repeatU;
        newObj.repeat.s1 = curObj.m_primitive.material.textureMap->repeatV ;
        newObj.isUsed = (cl_int)curObj.m_primitive.material.textureMap->isUsed;
        newObj.diffuse = copySceneColor( curObj.m_primitive.material.cDiffuse );
        newObj.ambient = copySceneColor( curObj.m_primitive.material.cAmbient );
        newObj.reflective = copySceneColor( curObj.m_primitive.material.cReflective );
        newObj.specular = copySceneColor( curObj.m_primitive.material.cSpecular );
        newObj.transparent = copySceneColor( curObj.m_primitive.material.cTransparent );
        newObj.emmisive = copySceneColor( curObj.m_primitive.material.cEmissive );
        newObj.shininess = curObj.m_primitive.material.shininess;
        newObj.ior = curObj.m_primitive.material.ior;
    }

    assert( m_objects.size() == objects.size() );

    QMap<QString, TexInfo> texMap = scene->getTexMap();

    m_textureHandles.resize(texMap.size());
    QMap<QString, TexInfo>::iterator iter =texMap.begin();
    for(int i = 0 ; iter != texMap.end(); iter++, i++ )
    {
        m_textureHandles[i] = (*iter).m_textureHandle;
    }

    assert( m_textureHandles.size() == texMap.size() );
}

void GPURayScene::setKernelArgs()
{
    assert( m_cl->m_kernelRay );

    cl_int ciErrNum;

    // Reset argument, necessary?
       cl_uint width = m_screenWidth;
       cl_uint height =m_screenHeight;
       cl_int lightCount = m_lightData.size();
       cl_int objectCount = m_objects.size();

       ciErrNum = clSetKernelArg(m_cl->m_kernelRay, 0, sizeof(cl_mem), (void*)&m_cl->m_cmPbo  );
       ciErrNum |= clSetKernelArg( m_cl->m_kernelRay, 1,  sizeof( cl_uint ), (void*)&width  );
       ciErrNum |= clSetKernelArg( m_cl->m_kernelRay, 2,  sizeof( cl_uint ), (void*)&height  );
       ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 4, sizeof(cl_mem), (void*)&m_cmLight );
       ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 5, sizeof(cl_int), (void*)&lightCount );
       ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 6, sizeof(cl_mem), (void*)&m_cmObject );
       ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 7, sizeof(cl_int), (void*)&objectCount );
        ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 8, sizeof(cl_float4), (void*)&m_global );
        for( int i = 11; i < 15; i++ )
        {
            ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, i, sizeof(cl_mem), (void*)&m_textures[i-11] );
        }

        if( ciErrNum != CL_SUCCESS )
        {
            cerr<<"Set kernel arguments failed in line: "<<__LINE__<<" File: "<<__FILE__<<endl;
            return;
        }
}

void GPURayScene::displayScreenTex()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, m_screenTex);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode( GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, m_screenWidth, m_screenHeight );

    glFrontFace( GL_CCW );
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(-1.0, -1.0, 0.5);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, -1.0, 0.5);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.5);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.5);

    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    //glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Re-enable lighting and depth
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_LIGHTING );
}

void GPURayScene::updateScreenTexFromPBO( )
{
    glBindTexture(GL_TEXTURE_2D, m_screenTex );
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, m_screenPbo );
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screenWidth, m_screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

cl_float4 GPURayScene::copySceneColor( CS123SceneColor color )
{
    cl_float4 result;
    result.s0 = color.channels[0];
    result.s1 = color.channels[1];
    result.s2 = color.channels[2];
    result.s3 = color.channels[3];
    return result;
}

cl_float16 GPURayScene::copySceneMatrix( Matrix4x4 mat )
{
    cl_float16 result;
    result.s0 = mat.data[0];
    result.s1 = mat.data[1];
    result.s2 = mat.data[2];
    result.s3 = mat.data[3];
    result.s4 = mat.data[4];
    result.s5 = mat.data[5];
    result.s6 = mat.data[6];
    result.s7 = mat.data[7];
    result.s8 = mat.data[8];
    result.s9 = mat.data[9];
    result.sa = mat.data[10];
    result.sb = mat.data[11];
    result.sc = mat.data[12];
    result.sd = mat.data[13];
    result.se = mat.data[14];
    result.sf = mat.data[15];
    return result;
}

cl_float4 GPURayScene::copySceneVector4( Vector4 v )
{
    cl_float4 result;
    result.s0 = v.data[0];
    result.s1 = v.data[1];
    result.s2 = v.data[2];
    result.s3 = v.data[3];
    return result;
}

cl_float3 GPURayScene::copySceneVector3( Vector3 v )
{
    cl_float3 result;
    result.s0 = v.xyz[0];
    result.s1 = v.xyz[1];
    result.s2 = v.xyz[2];
    return result;
}

void GPURayScene::syncGlobalSettings()
{
  //  m_globalSetting.useShadow = (cl_int)settings.useShadow;
    m_globalSetting.useShadow = (cl_int)settings.useShadow;
    m_globalSetting.usePointLight = (cl_int)settings.usePointLights;
    m_globalSetting.useDirectionalLight = (cl_int)settings.useDirectionalLights;
    m_globalSetting.useSpotLight = (cl_int)settings.useSpotLights;
    m_globalSetting.showTexture = (cl_int)settings.showTexture;
    m_globalSetting.useSupersampling = (cl_int)settings.useSupersampling;
    m_globalSetting.traceNum = (cl_int)settings.traceRaycursion;
    m_globalSetting.useReflection = (cl_int)settings.useReflection;

    if( m_cmGlobal )
    {
        clReleaseMemObject( m_cmGlobal );
    }
     cl_int ciErrNum;
    m_cmGlobal = clCreateBuffer(m_cl->m_context,
                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               sizeof(GlobalSettingHost),
                               &m_globalSetting,
                               &ciErrNum);

    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 3, sizeof(cl_mem), (void*)&m_cmGlobal );
    if( ciErrNum != CL_SUCCESS )
    {
        cerr<<"Set global setting failed in line:"<<__LINE__<<", File:"<<__FILE__<<endl;
        return ;
    }
}

void GPURayScene::initCLBuffers()
{
    assert( m_cl->m_context );
    cl_int ciErrNum1, ciErrNum2, ciErrNum3;
    m_cmLight  = clCreateBuffer(m_cl->m_context,
                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               m_lightData.size()*sizeof(LightDataHost),
                               m_lightData.data(),
                               &ciErrNum1);
    m_cmObject= clCreateBuffer(m_cl->m_context,
                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               m_objects.size()*sizeof(ObjectDataHost),
                               m_objects.data(),
                               &ciErrNum2);

    if( ciErrNum1 | ciErrNum2  != CL_SUCCESS )
    {
        cerr<<"Create buffer failed in line "<<__LINE__<<" File:"<<__FILE__<<endl;
        return;
    }
    int i = 0;
    for( ; i < 4 && i < m_textureHandles.size(); i++ )
    {
        m_textures[i] = clCreateFromGLTexture2D(m_cl->m_context, CL_MEM_READ_ONLY,
                                                GL_TEXTURE_2D, 0, m_textureHandles[i], &ciErrNum1);

         if( ciErrNum1!= CL_SUCCESS )
        {
            cerr<<"Create texture from GL texture failed in line "<<__LINE__<<" File:"<<__FILE__<<endl;
            return;
        }
    }

    if( i < 4 )
    {
        for( ; i < 4 ; i++ )
        {
            cl_image_format image_format;
            image_format.image_channel_order = CL_RGBA;
            image_format.image_channel_data_type = CL_UNSIGNED_INT8;
            m_textures[i] = clCreateImage2D(m_cl->m_context, CL_MEM_READ_ONLY,
                                               &image_format, 1, 1, 0, NULL, &ciErrNum1);

            if( ciErrNum1 != CL_SUCCESS )
            {
                cerr<<"Create image failed in line "<<__LINE__<<" File:"<<__FILE__<<endl;
                return;
            }
        }
    }
}
