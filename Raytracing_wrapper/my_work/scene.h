/*!
    @file scene.h
    @desc: declarations of Scene, TexInfo and SceneObject class
    @author: yanli
    @date: May 2013
 */

#ifndef SCENE_H
#define SCENE_H

#include "CS123SceneData.h"
#include "aabb.h"
#include <qgl.h>
#include <QHash>

class KdTree;
class KdTreeNode;
class View3D;
class Camera;
class CS123ISceneParser;
struct VboHandles;

/**
 * @struct: TexInfo
 * @brief The TexInfo struct is responsible for storing texture information
 */
struct TexInfo
{

    int m_mapIndex; // Map index
    GLuint m_textureHandle; // GL handle
    unsigned *m_texPointer; // Point to the actual data of texture
    int m_texWidth; // The width of texture;
    int m_texHeight; // The height of texture
};

/**
 * @class: SceneObject
 * @brief The SceneObject class is used for store the data for an primitive
 */
class SceneObject
{
public:

    SceneObject();
    ~SceneObject();

    Matrix4x4 m_transform; // Transformation matrix
    Matrix4x4 m_invTransform; // Inverse of transformation matrix
    Matrix4x4 m_invTTransformWithoutTrans; // Inverse of transformation
                                           // without translation
    CS123ScenePrimitive m_primitive; // Primitive structure
    AABB m_boundingBox; // Bounding box
    int m_textureMapID; // Texture map ID
    int m_arrayID; // Id in array
    TexInfo m_texture; // Texture info
};

/**
 * @class: Scene
 * @brief The Scene class is the holder for all kinds of data in the scene
 */
class Scene
{
public:

    Scene();
    Scene(Scene& s);
    virtual ~Scene();

    /**
     * @brief render: render the scene using info from context
     * @param context: the pointer to View3D
     */
    void render(View3D* context);

    /**
     * @brief parse: parse a scene using parser
     * @param sceneToFill: the scene to fill in
     * @param parser: the pointer to the parser
     */
    static void parse(Scene *sceneToFill, CS123ISceneParser *parser);

    /**
     * @brief recursiveParseNode: do recursive parsing
     * @param sceneToFill: the scene to fill in
     * @param node: the current scene node
     * @param transform: the transformation matrix
     */
    static void recursiveParseNode(Scene *sceneToFill, CS123SceneNode *node,
                                   Matrix4x4 transform);

    /**
     * Getters
     */
    const CS123SceneGlobalData & getGlobal() { return m_globalData; }
    const QList<CS123SceneLightData> & getLight() { return m_lightData; }
    const QVector<SceneObject> & getObjects() { return m_objects; }
    QVector<SceneObject>* getObjectPointer() { return &m_objects; }

    QVector<SceneObject*> getObjectPointers()
    {
        QVector<SceneObject*> result;
        for (int i = 0; i < m_objects.size(); i++)
            result.push_back(&m_objects[i]);

        return result;
    }

    QMap<int, TexInfo> getTexMap(){return m_textureMap; }

    AABB getExtends(){ return m_extends; }

    KdTree* getKdTree(){ return m_tree; }

    /**
     * @brief setLights: wrapper for setting lights
     */
    void setLights();

    /**
     * @brief initExtends
     */
    void initExtends();

    /**
     * @brief buildKdTree: wrapper for building kdtree
     */
    void buildKdTree();

    /**
     * @brief dumpKdTree: wrapper for dumping kdtree information
     */
    void dumpKdTree();

protected:

    /**
     * @brief addPrimitive
     * @param scenePrimitive
     * @param matrix
     */
    virtual void addPrimitive(const CS123ScenePrimitive &scenePrimitive,
                              const Matrix4x4 matrix);

    /**
     * @brief addLight
     * @param sceneLight
     */
    virtual void addLight(const CS123SceneLightData &sceneLight);

    /**
     * @brief setGlobal
     * @param global
     */
    virtual void setGlobal(const CS123SceneGlobalData &global);

    CS123SceneGlobalData m_globalData; // Global scene data
    QList<CS123SceneLightData> m_lightData; // List of light data
    QVector<SceneObject> m_objects; // List of objects
    QMap<int, TexInfo> m_textureMap; // Map for texture
    QMap<QString, TexInfo> m_texInfoMap; // Map for texture information
    int m_mapEnd; // The number of items of map
    AABB m_extends; // Bounding box for the scene
    KdTree* m_tree; // Pointer to the kdtree

private:

    /**
     * @brief drawPrimitive: draw a primitive
     * @param type: primitive type
     * @param vbos: pointer to the vbo handles
     * @param texHandle: pointer to the texture handle
     */
    void drawPrimitive(PrimitiveType type, const VboHandles* vbos,
                       GLuint texHandle);

    /**
     * @brief drawPrimitiveNormals: draw primitive normals
     * @param type: primitive type
     * @param vbos: the pointer to the vbo
     */
    void drawPrimitiveNormals(PrimitiveType type, const VboHandles* vbos);

    /**
     * @brief renderGeometry: render the geometry
     * @param useMaterials: use materials or not
     * @param vbos: the pointer to the vbo handles
     */
    void renderGeometry(const bool useMaterials, const VboHandles* vbos);

    /**
     * @brief applyMaterial: apply material to current rendering pipeline
     * @param material: material
     */
    void applyMaterial(const CS123SceneMaterial &material);

    /**
     * @brief setLight: set light in the scene
     * @param light: the light data
     */
    void setLight(const CS123SceneLightData &light);

    /**
     * @brief renderNormals: render normals
     * @param vbos: the pointer to the vbo handles
     *
     */
    void renderNormals(const VboHandles* vbos);

    /**
     * @brief renderKdTree: render the kdtree
     */
    void renderKdTree();

    /**
     * @brief renderKdTreeLeaf: render the kdtree's leaf
     * @param node: the pointer to the kdtree node
     */
    void renderKdTreeLeaf(KdTreeNode* node);
};

#endif // SCENE_H
