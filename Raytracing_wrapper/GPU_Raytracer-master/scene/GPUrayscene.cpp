/*!
    @file GPURayScene.cpp
    @desc: the definition for GPURayScene class
    @author: yanli
    @date: May 2013
 */

#include "GPUrayscene.h"
#include "view3d.h"
#include "global.h"
#include "camera.h"
#include "kdtree.h"

using std::endl;

extern "C" {
    void glBindBuffer (GLenum target, GLuint buffer);
    void glGenBuffers (GLsizei n, GLuint *buffers);
    void glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data,
                       GLenum usage);
    void glBufferSubData(GLenum target,GLintptr offset, GLsizeiptr size,
                          const GLvoid * data);
    void glGetBufferParameteriv(GLenum target, GLenum value, GLint * data);
    void glDeleteBuffers(GLsizei n, const GLuint* buffers);
}

GPURayScene::GPURayScene()
{
    m_cl               = NULL;
    m_cmGlobal         = NULL;
    m_cmLight          = NULL;
    m_cmObject         = NULL;
    m_cmOffsetBuffer   = NULL;
    m_cmTexPixelBuffer = NULL;
    m_cmKdNodes        = NULL;
    m_cmObjNodes       = NULL;
    m_pixels           = NULL;
    m_pixelNum         = 0;
}

GPURayScene::GPURayScene(CLPack* cl,
                          OrbitCamera* camera,
                          Scene* scene,
                          GLuint screenTexHandle,
                          GLuint screenPbo,
                          GLuint screenWidth,
                          GLuint screenHeight)
{
    m_cmGlobal         = NULL;
    m_cmLight          = NULL;
    m_cmObject         = NULL;
    m_cmOffsetBuffer   = NULL;
    m_cmTexPixelBuffer = NULL;
    m_cmKdNodes        = NULL;
    m_cmObjNodes       = NULL;
    m_pixelNum         = 0;
    m_pixels           = NULL;

    pushSceneData(scene);

    m_cl           = cl;
    m_camera       = camera;
    m_screenTex    = screenTexHandle;
    m_screenPbo    = screenPbo;
    m_screenWidth  = screenWidth;
    m_screenHeight = screenHeight;

    initCLBuffers();
    setKernelArgs();
    syncGlobalSettings();
}

GPURayScene::~GPURayScene()
{
    if (m_cmGlobal)
        clReleaseMemObject(m_cmGlobal);

    if (m_cmLight)
        clReleaseMemObject(m_cmLight);

    if (m_cmObject)
        clReleaseMemObject(m_cmObject);

    if (m_cmTexPixelBuffer)
        clReleaseMemObject(m_cmTexPixelBuffer);

    if (m_cmKdNodes)
        clReleaseMemObject(m_cmKdNodes);

    if (m_cmObjNodes)
        clReleaseMemObject(m_cmObjNodes);

    if (m_pixels)
        delete []m_pixels;
}

void GPURayScene::render()
{
    cl_int ciErrNum;
    // Sync gl calls
    glFinish();

    Vector4 eyePos   = m_camera->getEyePos();
    Matrix4x4 invMat = m_camera->getInvViewTransMatrix();

    cl_float16 clInvMat = copyMatrix(invMat);
    cl_float4 clEyePos  = copyVector4(eyePos);
    cl_float clEyeNear  = m_camera->getNear();

    ciErrNum  = clSetKernelArg(m_cl->m_kernelRay, 9, sizeof(cl_float4),
                               (void*)&clEyePos);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 10, sizeof(cl_float),
                               (void*)&clEyeNear);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 11, sizeof(cl_float16),
                               (void*)&clInvMat);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Set kernel failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        return;
    }

    cl_mem canvas_dev = clCreateFromGLTexture2D(m_cl->m_context,
                                                CL_MEM_WRITE_ONLY,
                                                GL_TEXTURE_2D,
                                                0,
                                                m_objects[0].texHandle,
                                                &ciErrNum);
    cl_image_format format;

    clGetImageInfo(canvas_dev,
                   CL_IMAGE_FORMAT,
                   sizeof(cl_image_format),
                   &format,
                   NULL);
    // Before using GL object we need to acquire
    ciErrNum =  clEnqueueAcquireGLObjects(m_cl->m_queue,
                                          1,
                                          &m_cl->m_cmPbo,
                                          0,
                                          NULL,
                                          NULL);
    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Acquire GL objects failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        return ;
    }

    // Launch the kernel
    ciErrNum  = clEnqueueNDRangeKernel(m_cl->m_queue,
                                       m_cl->m_kernelRay,
                                       2,
                                       NULL,
                                       m_cl->m_globalWorkSize,
                                       m_cl->m_localWorkSize,
                                       0,
                                       NULL,
                                       NULL);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Launch kernel failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        return ;
    }

    clFinish(m_cl->m_queue);
    // Release buffer then GL can use
    clEnqueueReleaseGLObjects(m_cl->m_queue, 1, &m_cl->m_cmPbo, 0, NULL, NULL);

    // Sync
    clFinish(m_cl->m_queue);
    QVector<LightDataHost> test;
    test.resize(m_lightData.size());

    // Now read back the buffer to texture
    updateScreenTexFromPBO();
    displayScreenTex();
}

void GPURayScene::pushSceneData(Scene* scene)
{
    const CS123SceneGlobalData global = scene->getGlobal();
    m_global.s0 = global.ka;
    m_global.s1 = global.kd;
    m_global.s2 = global.ks;
    m_global.s3 = global.kt;

    const QList<CS123SceneLightData> lights = scene->getLight();

    m_lightData.resize(lights.size());

    for (int i = 0; i < lights.size(); i++)
    {
        CS123SceneLightData curLight = lights[i];
        LightDataHost& newlight = m_lightData[i];

        newlight.id       = curLight.id;
        newlight.type     = (cl_int)curLight.type;
        newlight.color    = copyColor(curLight.color);
        newlight.function = copyVector3(curLight.function);
        newlight.pos      = copyVector4(curLight.pos);
        newlight.dir      = copyVector4(curLight.dir);
        newlight.radius   = curLight.radius;
        newlight.penumbra = curLight.penumbra;
        newlight.angle    = curLight.angle;
        newlight.width    = curLight.width;
        newlight.height   = curLight.height;
    }
    assert(m_lightData.size() == lights.size());

    const QVector<SceneObject> objects = scene->getObjects();
    m_objects.resize(objects.size());

    for (int i = 0; i < objects.size(); i++)
    {
        SceneObject curObj = objects[i];
        ObjectDataHost& newObj = m_objects[i];

        newObj.transform        = copyMatrix(curObj.m_transform);
        newObj.invTransform     = copyMatrix(curObj.m_invTransform);
        newObj.invTWithoutTrans = copyMatrix(curObj.m_invTTransformWithoutTrans);
        newObj.type             = (cl_int)curObj.m_primitive.type;
        newObj.texHandle        = (cl_int)curObj.m_texture.m_textureHandle;
        newObj.texMapID         = (cl_int)curObj.m_texture.m_mapIndex;
        newObj.texWidth         = (cl_int)curObj.m_texture.m_texWidth;
        newObj.texHeight        = (cl_int)curObj.m_texture.m_texHeight;
        newObj.texBlend         = curObj.m_primitive.material.blend;
        newObj.texRepeat.s0     = curObj.m_primitive.material.textureMap->repeatU;
        newObj.texRepeat.s1     = curObj.m_primitive.material.textureMap->repeatV ;
        newObj.texIsUsed        = (cl_int)curObj.m_primitive.material.textureMap->isUsed;
        newObj.diffuse          = copyColor(curObj.m_primitive.material.cDiffuse);
        newObj.ambient          = copyColor(curObj.m_primitive.material.cAmbient);
        newObj.reflective       = copyColor(curObj.m_primitive.material.cReflective);
        newObj.specular         = copyColor(curObj.m_primitive.material.cSpecular);
        newObj.transparent      = copyColor(curObj.m_primitive.material.cTransparent);
        newObj.emmisive         = copyColor(curObj.m_primitive.material.cEmissive);
        newObj.shininess        = curObj.m_primitive.material.shininess;
        newObj.ior              = curObj.m_primitive.material.ior;
    }

    assert(m_objects.size() == objects.size());

    copyKdTree(scene->getKdTree());

    QMap<int, TexInfo> texMap = scene->getTexMap();

    if (texMap.size() == 0)
        return;

    m_textureHandles.resize(texMap.size());
    m_textureOffsets.resize(texMap.size());

    QMap<int, TexInfo>::iterator iter = texMap.begin();

    cl_uint offsetBegin = 0;

    for (int i = 0 ; iter != texMap.end(); iter++, i++)
    {
        m_textureHandles[i] = (*iter).m_textureHandle;
        m_textureOffsets[i] = offsetBegin;
        offsetBegin += (*iter).m_texWidth * (*iter).m_texHeight;
    }
    m_pixelNum = offsetBegin;
    if (m_pixelNum != 0)
        m_pixels = (cl_uint*)malloc(sizeof(cl_uint) * m_pixelNum);

    iter = texMap.begin();
    for (int i = 0 ; iter != texMap.end(); iter++, i++)
    {
        memcpy(m_pixels+m_textureOffsets[i],
               (*iter).m_texPointer,
               sizeof(unsigned)*(*iter).m_texWidth*(*iter).m_texHeight);
    }
}

void GPURayScene::setKernelArgs()
{
    assert(m_cl->m_kernelRay);

    cl_int ciErrNum;
    cl_uint width       = m_screenWidth;
    cl_uint height      = m_screenHeight;
    cl_int lightCount   = m_lightData.size();
    cl_int objectCount  = m_objects.size();
    cl_int offsetCount  = m_textureOffsets.size();
    cl_int kdnodeCount  = m_kdNodes.size();
    cl_int objnodeCount = m_objNodes.size();

    ciErrNum  = clSetKernelArg(m_cl->m_kernelRay, 0, sizeof(cl_mem),
                               (void*)&m_cl->m_cmPbo);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 1,  sizeof(cl_uint),
                               (void*)&width);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 2,  sizeof(cl_uint),
                               (void*)&height);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 4, sizeof(cl_mem),
                               (void*)&m_cmLight);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 5, sizeof(cl_int),
                               (void*)&lightCount);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 6, sizeof(cl_mem),
                               (void*)&m_cmObject);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 7, sizeof(cl_int),
                               (void*)&objectCount);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 8, sizeof(cl_float4),
                               (void*)&m_global);

    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 12, sizeof(cl_mem),
                               (void*)&m_cmTexPixelBuffer);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 13,sizeof(cl_int),
                               (void*)&m_pixelNum);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 14, sizeof(cl_mem),
                               (void*)&m_cmOffsetBuffer);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 15, sizeof(cl_uint),
                               (void*)&offsetCount);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 16, sizeof(cl_mem),
                               (void*)&m_cmKdNodes);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 17, sizeof(cl_uint),
                               (void*)&kdnodeCount);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 18, sizeof(cl_mem),
                               (void*)&m_cmObjNodes);
    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay, 19, sizeof(cl_uint),
                               (void*)&objnodeCount);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr<<"Set kernel arguments failed in line: "<< __LINE__ << " File: "
            << __FILE__ << endl;
        return;
    }
}

void GPURayScene::displayScreenTex()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_screenTex);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, m_screenWidth, m_screenHeight);

    glFrontFace(GL_CCW);
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

    glBindTexture(GL_TEXTURE_2D, 0);

    // Re-enable lighting and depth
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void GPURayScene::updateScreenTexFromPBO()
{
    glBindTexture(GL_TEXTURE_2D, m_screenTex);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, m_screenPbo);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screenWidth, m_screenHeight,
                    GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

cl_float4 GPURayScene::copyColor(CS123SceneColor color)
{
    cl_float4 result;
    result.s0 = color.channels[0];
    result.s1 = color.channels[1];
    result.s2 = color.channels[2];
    result.s3 = color.channels[3];
    return result;
}

cl_float16 GPURayScene::copyMatrix(Matrix4x4 mat)
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

cl_float4 GPURayScene::copyVector4(Vector4 v)
{
    cl_float4 result;
    result.s0 = v.data[0];
    result.s1 = v.data[1];
    result.s2 = v.data[2];
    result.s3 = v.data[3];
    return result;
}

cl_float3 GPURayScene::copyVector3(Vector3 v)
{
    cl_float3 result;
    result.s0 = v.xyz[0];
    result.s1 = v.xyz[1];
    result.s2 = v.xyz[2];
    return result;
}

void GPURayScene::copyKdTree(KdTree* tree)
{
    m_kdNodes.resize(KDTREE_ARRAY_SIZE);
    m_objNodes.resize(KDTREE_ARRAY_SIZE);

    KdTreeNode* kdNodeArray = tree->getKdTreeNodeArray();
    ObjectNode* objNodeArray = tree->getObjectNodeArray();

    for (int i = 0; i < KDTREE_ARRAY_SIZE; i++)
    {
        m_kdNodes[i].axis     = kdNodeArray[i].getAxis();
        m_kdNodes[i].boxBegin = copyVector3(kdNodeArray[i].getAABB().getPos());
        m_kdNodes[i].boxSize  = copyVector3(kdNodeArray[i].getAABB().getSize());
        m_kdNodes[i].leaf     = kdNodeArray[i].isLeaf();
        m_kdNodes[i].split    = kdNodeArray[i].getSplitPos();

        if (kdNodeArray[i].getLeft())
            m_kdNodes[i].leftIndex = kdNodeArray[i].getLeft() - kdNodeArray;
        else
            m_kdNodes[i].leftIndex = -1;

        if (kdNodeArray[i].getRight())
            m_kdNodes[i].rightIndex = kdNodeArray[i].getRight() - kdNodeArray;
        else
            m_kdNodes[i].rightIndex = -1;

        if (kdNodeArray[i].getObjectList())
            m_kdNodes[i].objIndex = kdNodeArray[i].getObjectList() - objNodeArray;
        else
            m_kdNodes[i].objIndex = -1;

        if (objNodeArray[i].getObject())
            m_objNodes[i].objectIndex = objNodeArray[i].getObject()->m_arrayID;
        else
            m_objNodes[i].objectIndex = -1;

        if (objNodeArray[i].getNext())
            m_objNodes[i].nextNodeIndex = objNodeArray[i].getNext() - objNodeArray;
        else
            m_objNodes[i].nextNodeIndex = -1;
    }

    kdnode_test  = m_kdNodes;
    objnode_test = m_objNodes;
    dumpCLKdTree("./output/clkdtree.txt");
}

void GPURayScene::dumpCLKdTree(std::string fileName)
{
    assert(m_kdNodes.size() > 0 && m_objNodes.size() > 0);
    std::ofstream out(fileName.c_str());

    if (!out.is_open())
    {
        cerr << "Cannot open file kdtree.txt" << endl;
        return;
    }
    // The first one is the root
    KdTreeNodeHost root = m_kdNodes[0];
    out << "Extends: " << endl << (float)root.boxBegin.x << " "
        << (float)root.boxBegin.y << " " << (float)root.boxBegin.z << endl;
    out << (float)(root.boxBegin.x + root.boxSize.x) << " "
        << (float)(root.boxBegin.y + root.boxSize.y) << " "
        << (float)(root.boxBegin.z + root.boxSize.z) << endl;

    dumpCLKdTreeInfoLeaf(0, out, 0);
}

void GPURayScene::dumpCLKdTreeInfoLeaf(int curIndex,
                                       std::ofstream& ofile,
                                       int depth)
{
    for (int i = 0; i < depth; i++)
    {
        ofile << "    ";
    }
    KdTreeNodeHost cur = m_kdNodes[curIndex];
    ofile << "Node:   " << " depth " << depth
          <<" Leaf " << (cur.leaf ? "true ":"false ")
          <<" axis " << cur.axis << " splitpos " << cur.split
          << endl << endl;

    if (cur.leaf)
    {
        // If it is a leaf, dump the objects
        int objIndex = cur.objIndex;

        ofile<<" obj list: ";
        while (objIndex != -1)
        {
            ObjectNodeHost obj = m_objNodes[objIndex];
            ofile << "(" << m_objects[obj.objectIndex].type << ") ";
            objIndex = obj.nextNodeIndex;
        }

        return;
    }
    if (cur.leftIndex != -1)
    {
        dumpCLKdTreeInfoLeaf(cur.leftIndex, ofile, depth + 1);
    }
    if (cur.rightIndex != -1)
    {
        dumpCLKdTreeInfoLeaf(cur.rightIndex, ofile, depth + 1);
    }
}


void GPURayScene::syncGlobalSettings()
{
    m_globalSetting.useShadow           = (cl_int)settings.useShadow;
    m_globalSetting.usePointLight       = (cl_int)settings.usePointLights;
    m_globalSetting.useDirectionalLight = (cl_int)settings.useDirectionalLights;
    m_globalSetting.useSpotLight        = (cl_int)settings.useSpotLights;
    m_globalSetting.showTexture         = (cl_int)settings.showTexture;
    m_globalSetting.useSupersampling    = (cl_int)settings.useSupersampling;
    m_globalSetting.traceNum            = (cl_int)settings.traceRaycursion;
    m_globalSetting.useReflection       = (cl_int)settings.useReflection;
    m_globalSetting.useKdTree           = (cl_int)settings.useKdTree;

    if (m_cmGlobal)
    {
        clReleaseMemObject(m_cmGlobal);
    }
    cl_int ciErrNum;
    m_cmGlobal = clCreateBuffer(m_cl->m_context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(GlobalSettingHost),
                                &m_globalSetting,
                                &ciErrNum);

    ciErrNum |= clSetKernelArg(m_cl->m_kernelRay,
                               3,
                               sizeof(cl_mem),
                               (void*)&m_cmGlobal);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Set global setting failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        return;
    }
}

void GPURayScene::initCLBuffers()
{
    assert(m_cl->m_context);

    cl_int ciErrNum1, ciErrNum2, ciErrNum3,ciErrNum4, ciErrNum5, ciErrNum6;

    m_cmLight  = clCreateBuffer(m_cl->m_context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                m_lightData.size() * sizeof(LightDataHost),
                                m_lightData.data(),
                                &ciErrNum1);

    m_cmObject= clCreateBuffer(m_cl->m_context,
                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               m_objects.size() * sizeof(ObjectDataHost),
                               m_objects.data(),
                               &ciErrNum2);

    if (m_pixels)
    {
        m_cmTexPixelBuffer= clCreateBuffer(m_cl->m_context,
                                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           m_pixelNum * sizeof(cl_uint),
                                           m_pixels,
                                           &ciErrNum3);
    }
    else
    {
        ciErrNum3 = CL_SUCCESS;
    }

    if (m_textureOffsets.size())
    {
        m_cmOffsetBuffer = clCreateBuffer(m_cl->m_context,
                                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          m_textureOffsets.size() * sizeof(cl_uint),
                                          m_textureOffsets.data(),
                                          &ciErrNum4);
    }
    else
    {
        ciErrNum4 = CL_SUCCESS;
    }

    m_cmKdNodes = clCreateBuffer(m_cl->m_context,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 m_kdNodes.size() * sizeof(KdTreeNodeHost),
                                 m_kdNodes.data(),
                                 &ciErrNum5);
    m_cmObjNodes = clCreateBuffer(m_cl->m_context,
                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  m_objNodes.size() * sizeof(ObjectNodeHost),
                                  m_objNodes.data(),
                                  &ciErrNum6);

    if (m_pixels)
    {
        delete []m_pixels;
        m_pixels = NULL;
    }

    if (ciErrNum1 |
            ciErrNum2 |
            ciErrNum3 |
            ciErrNum4 |
            ciErrNum5 |
            (ciErrNum6 != CL_SUCCESS))
    {
        cerr << "Create buffer failed in line " << __LINE__ << " File:"
             << __FILE__ << endl;
        return;
    }
}
