/*!
    @file scene.cpp
    @desc: definitions of Scene, TexInfo and SceneObject class
    @author: yanli
    @date: May 2013
 */

#include "scene.h"
#include "camera.h"
#include "view3d.h"
#include "shape_draw.h"
#include "CS123ISceneParser.h"
#include "resource_loader.h"
#include "kdtree.h"

SceneObject::SceneObject()
{

    m_primitive.type                = PRIMITIVE_NONE;
    m_primitive.material.textureMap = NULL;
    m_primitive.material.bumpMap    = NULL;
    m_texture.m_textureHandle       = 0;
    m_texture.m_texPointer          = NULL;
    m_texture.m_texWidth            = 0;
    m_texture.m_texHeight           = 0;
}

SceneObject::~SceneObject()
{
    // We don't need to delete the pointer because it
    // will be freed by XML parser
}

Scene::Scene()
{

    m_mapEnd = 0;
    m_tree   = NULL;
}

Scene::Scene(Scene& s)
{

    m_globalData = s.m_globalData;
    m_lightData  = s.m_lightData;
    m_objects    = s.m_objects;
    m_mapEnd     = 0;
    m_tree       = NULL;
}

Scene::~Scene()
{

    // Release gl textures
    for (int i = 0; i < m_objects.size(); i++)
    {
        if (m_objects[i].m_texture.m_textureHandle)
            glDeleteTextures(1, &m_objects[i].m_texture.m_textureHandle);
    }

    QMap<int, TexInfo>::iterator iter = m_textureMap.begin();

    // Release the textures, it's pointer to the actual data
    for (;iter != m_textureMap.end(); iter++)
    {
        if ((*iter).m_texPointer)
            delete []((*iter).m_texPointer);
    }

   // Release the kdtree
   if (m_tree)
       delete m_tree;
}

void Scene::render(View3D *context)
{

    // Get the active camera
    OrbitCamera *camera = context->getCamera();
    assert(camera);

    // Apply camera
    camera->update();

    const VboHandles* vbos = context->getVBOs();
    assert(vbos);

    setLights();

    float red[3] = {1,0,0};
    // Show bounding box?
    if (settings.showBoundingBox)
        drawAABB(m_extends, red);

    // Show kdtree?
    if (settings.showKdTree && settings.useKdTree)
        renderKdTree();

    // Fill the geometry with white
    glColor3f(1, 1, 1);

    if (settings.useLighting)
    {
        glDisable(GL_COLOR_MATERIAL);
        glEnable(GL_LIGHTING);
    }

    // Render geometry
    renderGeometry(true, vbos);

    if (settings.useLighting)
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
    }

    // Outline the geometry with black
    if (settings.drawWireFrame)
    {
        glColor3f(0, 0, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderGeometry(false, vbos);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (settings.drawNormals)
    {
        // Calculate an eye position so we can do billboarded normals
        // Render normals in black
        glColor3f(0, 0, 0);
        renderNormals(vbos);
    }
}

void Scene::parse(Scene *sceneToFill, CS123ISceneParser *parser)
{

    sceneToFill->initExtends();
    CS123SceneNode* node = parser->getRootNode();
    Matrix4x4 transform = Matrix4x4::identity();

    // Do recursive parsing
    recursiveParseNode(sceneToFill, node, transform);

    CS123SceneGlobalData tempGlobalData;
    parser->getGlobalData(tempGlobalData);
    sceneToFill->setGlobal(tempGlobalData);

    for (int i = 0; i < parser->getNumLights(); i++)
    {
        CS123SceneLightData tempLightData;
        parser->getLightData(i, tempLightData);
        sceneToFill->addLight(tempLightData);
    }
}

void Scene::recursiveParseNode(Scene *sceneToFill,
                               CS123SceneNode *node,
                               Matrix4x4 transform)
{

    Matrix4x4 compositTrans = transform;
    for (unsigned int i = 0; i < node->transformations.size(); i++)
    {
        CS123SceneTransformation *tempTrans = node->transformations[i];
        Matrix4x4 trans = Matrix4x4::identity();
        switch (tempTrans->type)
        {
        case TRANSFORMATION_SCALE:
        {
            trans = getScaleMat(tempTrans->scale);
            break;
        }
        case TRANSFORMATION_TRANSLATE:
        {
            trans = getTransMat(tempTrans->translate);
            break;
        }
        case TRANSFORMATION_ROTATE:
        {
            Vector4 origin = Vector4(0,0,0,1);
            trans = getRotMat(origin, tempTrans->rotate, tempTrans->angle);
            break;
        }
        default:
        {
            trans = tempTrans->matrix;
            break;
        }
        }
        compositTrans *= trans;
    }

    for (unsigned int i = 0; i < node->primitives.size(); i++)
     {
        CS123ScenePrimitive *primitive = node->primitives[i];
        sceneToFill->addPrimitive((*primitive) , compositTrans);
     }


    for (unsigned int i = 0; i < node->children.size(); i++)
    {
        recursiveParseNode(sceneToFill, node->children[i], compositTrans);
    }
}

void Scene::drawPrimitive(PrimitiveType type,
                          const VboHandles* vbos,
                          GLuint texHandle)
{

    switch(type)
    {
    case PRIMITIVE_CUBE:
            drawCube(vbos->cubeVBO, vbos->cubeElementVBO, texHandle);
        break;
    case PRIMITIVE_CYLINDER:
            drawCylinder(vbos->cylinderVBO, vbos->cylinderElementVBO, texHandle);
        break;
    case PRIMITIVE_SPHERE:
            drawSphere(vbos->sphereVBO, vbos->sphereElementVBO, texHandle);
        break;
    case PRIMITIVE_CONE:
            drawCone(vbos->coneVBO, vbos->coneElementVBO, texHandle);
        break;
    case PRIMITIVE_MESH:
        break;
    case PRIMITIVE_TORUS:
        break;
    default:
        cerr<<"[Scene::drawPrimitive] Invalid type"<<endl;
        assert(0);
        break;
    }
}

void Scene::drawPrimitiveNormals(PrimitiveType type, const VboHandles* vbos)
{

    switch(type)
    {
    case PRIMITIVE_CUBE:
            drawNormals(vbos->cubeVBO, (cubeTess+1)*(cubeTess+1)*6);
        break;
    case PRIMITIVE_CYLINDER:
            drawNormals(vbos->cylinderVBO, (cylinderTess+1)*4);
        break;
    case PRIMITIVE_SPHERE:
            drawNormals(vbos->sphereVBO, (sphereTess[1]+1)*(sphereTess[0]+1));
        break;
    case PRIMITIVE_CONE:
            drawNormals(vbos->coneVBO, (coneTess+1)*3);
        break;
    default:
        cerr<<"[Scene::drawPrimitive] Invalid type"<<endl;
        assert(0);
        break;
    }
}

void Scene::renderGeometry(const bool useMaterials, const VboHandles* vbos)
{

    for (int i = 0; i < m_objects.size(); i++)
    {
        CS123ScenePrimitive primitive = m_objects[i].m_primitive;
        GLuint texHandle              = m_objects[i].m_texture.m_textureHandle;
        Matrix4x4 transform           = m_objects[i].m_transform;
        AABB boundingBox              = m_objects[i].m_boundingBox;

        float white[3] = {1,1,1};
        if (settings.showBoundingBox)
            drawAABB(boundingBox, white);

        transform = Matrix4x4::transpose(transform);
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_NORMALIZE);

        // Transform the object
        glPushMatrix();
        glMultMatrixf(transform.data);

        if (useMaterials)
        {
            CS123SceneMaterial tempMaterial = primitive.material;
            tempMaterial.cAmbient.b *= m_globalData.ka;
            tempMaterial.cAmbient.g *= m_globalData.ka;
            tempMaterial.cAmbient.r *= m_globalData.ka;
            tempMaterial.cDiffuse.b *= m_globalData.kd;
            tempMaterial.cDiffuse.g *= m_globalData.kd;
            tempMaterial.cDiffuse.r *= m_globalData.kd;
            tempMaterial.cSpecular.b *= m_globalData.ks;
            tempMaterial.cSpecular.g *= m_globalData.ks;
            tempMaterial.cSpecular.r *= m_globalData.ks;
            tempMaterial.cTransparent.b *= m_globalData.kt;
            tempMaterial.cTransparent.g *= m_globalData.kt;
            tempMaterial.cTransparent.r *= m_globalData.kt;

            applyMaterial(tempMaterial);
        }
        drawPrimitive(primitive.type, vbos, texHandle);

        if (settings.useLighting)
            glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
        // primitive is a shape

        glPopMatrix();
        glDisable(GL_NORMALIZE);
    }
}

void Scene::applyMaterial(const CS123SceneMaterial &material)
{

    // Make sure the members of CS123SceneColor are packed tightly
    COMPILE_TIME_ASSERT(sizeof(CS123SceneColor) == sizeof(float) * 4);

    if (settings.useLighting)
    {
        // Use materials when lighting is enabled
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &material.cAmbient.r);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &material.cDiffuse.r);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material.cSpecular.r);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &material.cEmissive.r);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);
    }
    else
    {
        // Materials don't work when lighting is disabled, use colors instead
        glColor3fv(&material.cDiffuse.r);
    }
}

void Scene::setLights()
{

    // Firstly disable all the lights
    for (int i = 0; i < 8;i++)
    {
        glDisable(GL_LIGHT0 + i);
    }

    for (int i  = 0; i < m_lightData.size(); i++)
    {
        setLight(m_lightData[i]);
    }
}

void Scene::initExtends()
{

    Vector3 pos  = Vector3(POS_INF, POS_INF, POS_INF);
    Vector3 size = Vector3(0,0,0);
    m_extends = AABB(pos, size);
}

void Scene::buildKdTree()
{

    m_tree = new KdTree();
    m_tree->init();
    m_tree->build(this);

    // Dump the kdtree info
    dumpKdTree();
}

void Scene::dumpKdTree()
{
    // Wrapper
    dumpKdTreeInfo(m_tree);
}

void Scene::setLight(const CS123SceneLightData &light)
{

    float zero[] = { 0, 0, 0, 0 };

    // There are 8 lights in OpenGL, GL_LIGHT0 to GL_LIGHT7, and
    // each GL_LIGHT* is one greater than the previous one.  This
    // means the nth light is GL_LIGHT0 + n.
    int id = GL_LIGHT0 + light.id;

    glLightfv(id, GL_AMBIENT, zero);
    glLightfv(id, GL_DIFFUSE, &light.color.r);
    glLightfv(id, GL_SPECULAR, &light.color.r);
    glLightf(id, GL_CONSTANT_ATTENUATION, light.function.xyz[0]);
    glLightf(id, GL_LINEAR_ATTENUATION, light.function.xyz[1]);
    glLightf(id, GL_QUADRATIC_ATTENUATION, light.function.xyz[2]);

    if (glIsEnabled(id))
        cout << "warning: GL_LIGHT"
             << light.id << " enabled more than once, not supposed to happen"
             << endl;

    switch (light.type)
    {
    case LIGHT_POINT:
    {
        // Convert from double[] to float[] and make sure the w coordinate is
        // correct (the CS123 scene loader gets it wrong for point lights)
        float position[] = { light.pos.data[0], light.pos.data[1],
                             light.pos.data[2], 1 };
        glLightfv(id, GL_POSITION, position);
        if (settings.usePointLights)
            glEnable(id);
        break;
    }

    case LIGHT_DIRECTIONAL:
    {
        // Convert from double[] to float[] and make sure the direction vector
        // is normalized (it isn't for a lot of scene files)
        Vector4 direction = -light.dir.getNormalized();
        float position[] = { direction.data[0], direction.data[1],
                             direction.data[2], direction.data[3] };
        glLightfv(id, GL_POSITION, position);
        if (settings.useDirectionalLights)
            glEnable(id);
        break;
    }

    case LIGHT_SPOT:
    {
        //spot light, need to set the cutoff, direction and exponent
        float position[] = { light.pos.data[0],
                             light.pos.data[1],
                             light.pos.data[2]
                           };

        glLightfv(id, GL_POSITION, position);
        float dir[] = { light.dir.data[0],
                        light.dir.data[1],
                        light.dir.data[2]
                      };


        glLightf(id, GL_SPOT_CUTOFF, light.penumbra);
        glLightf(id, GL_SPOT_EXPONENT, 5);
        glLightfv(id, GL_SPOT_DIRECTION, dir);

        // default set to EXPONENT_SPOT
        if (settings.useSpotLights)
            glEnable(id);
        break;
    }

    default:
        break;
    }
}

void Scene::renderNormals(const VboHandles* vbos)
{

    for (int i = 0; i < m_objects.size(); i++)
    {
        Matrix4x4 transform = m_objects[i].m_transform;
        transform = Matrix4x4::transpose(transform);
        glMatrixMode(GL_MODELVIEW);

        // We enable this because the normal may be scaled by the transform matrix
        glEnable(GL_NORMALIZE);

        // Transform the object
        glPushMatrix();
        glMultMatrixf(transform.data);
        drawPrimitiveNormals(m_objects[i].m_primitive.type, vbos);

        glPopMatrix();
        glDisable(GL_NORMALIZE);
    }
}

void Scene::renderKdTree()
{

    if (!m_tree)
        return;

    KdTreeNode* root = m_tree->getRoot();
    renderKdTreeLeaf(root);
}

void Scene::renderKdTreeLeaf(KdTreeNode* node)
{

    if (!node)
        return;

    // Recursive render
    if (!node->isLeaf())
    {
        float color[3] = {0.f, 1.f, 0.f};

        if (node->getLeft())
        {
            KdTreeNode* left = node->getLeft();
            AABB leftAABB = left->getAABB();
            drawAABB(leftAABB, color);
            renderKdTreeLeaf(left);
        }

        if (node->getRight())
        {
            KdTreeNode* right = node->getRight();
            AABB rightAABB = right->getAABB();
            drawAABB(rightAABB, color);
            renderKdTreeLeaf( right);
        }
    }
}

void Scene::addPrimitive(const CS123ScenePrimitive &scenePrimitive,
                         const Matrix4x4 matrix)
{

    SceneObject obj;
    obj.m_primitive    = scenePrimitive;
    obj.m_transform    = matrix;
    obj.m_invTransform = obj.m_transform.getInverse();

    Matrix4x4 compMat = obj.m_transform;
    compMat.data[3]   = 0;
    compMat.data[7]   = 0;
    compMat.data[11]  = 0;
    obj.m_invTTransformWithoutTrans = compMat.getInverse().getTranspose();

    // Compute the bounding box
    switch(obj.m_primitive.type)
    {
    case PRIMITIVE_CUBE:
        obj.m_boundingBox = computeCubeAABB(obj.m_transform);
        break;
    case PRIMITIVE_CONE:
        obj.m_boundingBox = computeConeAABB(obj.m_transform);
        break;
    case PRIMITIVE_CYLINDER:
        obj.m_boundingBox = computeCylinderAABB(obj.m_transform);
        break;
    case PRIMITIVE_SPHERE:
        obj.m_boundingBox = computeSphereAABB(obj.m_transform);
        break;
    case PRIMITIVE_TORUS:
    case PRIMITIVE_MESH:
    default:
        assert(0);
        break;
    }

    Vector3 bakPos =  m_extends.getPos() ;

    if (m_extends.getPos().x > obj.m_boundingBox.x())
        m_extends.getPos().x = obj.m_boundingBox.x();
    if (m_extends.getPos().y > obj.m_boundingBox.y())
        m_extends.getPos().y = obj.m_boundingBox.y();
    if (m_extends.getPos().z > obj.m_boundingBox.z())
        m_extends.getPos().z = obj.m_boundingBox.z();

 if (bakPos == Vector3(POS_INF, POS_INF, POS_INF))
     bakPos = m_extends.getPos();

    Vector3 max = obj.m_boundingBox.getPos() + obj.m_boundingBox.getSize();
    Vector3 extMax = bakPos + m_extends.getSize();

    if (extMax.x < max.x)
        extMax.x = max.x;
    if (extMax.y < max.y)
        extMax.y = max.y;
    if (extMax.z < max.z)
        extMax.z = max.z;

    m_extends.getSize() = extMax - m_extends.getPos();

    QString path = scenePrimitive.material.textureMap->filename.c_str();
    if (path.size() && scenePrimitive.material.textureMap->isUsed)
    {
        QMap<QString, TexInfo>::iterator iter = m_texInfoMap.find(path);
        if (iter != m_texInfoMap.end())
        {
            obj.m_texture = *iter;
        }
        else
        {
            obj.m_texture.m_textureHandle = loadTexture(path);

            int width = 0;
            int height = 0;
            glBindTexture(GL_TEXTURE_2D, obj.m_texture.m_textureHandle);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            unsigned* tex = new unsigned[width*height];
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
            glBindTexture(GL_TEXTURE_2D, 0);
            assert(width != 0 && height != 0);

            obj.m_texture.m_texPointer = tex;
            obj.m_texture.m_texHeight  = height;
            obj.m_texture.m_texWidth   = width;
            obj.m_texture.m_mapIndex   = m_mapEnd;

            m_textureMap.insert(obj.m_texture.m_mapIndex, obj.m_texture);
            m_texInfoMap.insert(path, obj.m_texture);
            // We pushed new value so we add this
            m_mapEnd++;
        }
    }
    obj.m_arrayID = m_objects.size();
    m_objects.append(obj);
}

void Scene::addLight(const CS123SceneLightData &sceneLight)
{

    m_lightData.append(sceneLight);
}

void Scene::setGlobal(const CS123SceneGlobalData &global)
{

    m_globalData = global;
}


