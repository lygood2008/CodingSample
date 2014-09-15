/*!
    @file GPURayScene.h
    @desc: scene holder for GPU ray tracing
    @author: yanli
    @date: May 2013
 */

#ifndef GPURayScene_H
#define GPURayScene_H


#include <QVector>
#include <QHash>
#include <string>
#include <fstream>
#include <qgl.h>
#include "CL/cl.h"
#include "oclUtils.h"
#include "scene.h"


class Scene;
struct CLPack;
class OrbitCamera;
class KdTree;

/**
 * @struct: KdTreeNodeHost
 * @brief The KdTreeNodeHost struct: host side structure for storing kdtree node
 */
struct KdTreeNodeHost
{
    cl_float3 boxBegin;
    cl_float3 boxSize;
    cl_float split;
    cl_int leaf;
    cl_int axis;
    cl_int leftIndex;
    cl_int rightIndex;
    cl_int objIndex;
};

/**
 * @struct: ObjectNodeHost
 * @brief The ObjectNodeHost struct: host side structure for storing obj node
 */
struct ObjectNodeHost
{
    cl_int objectIndex;
    cl_int nextNodeIndex;
};

/**
 * @class GPURayScene
 * @brief The GPURayScene class is used for managing everything used for OpenCL
 */
class GPURayScene
{
public:

    /**
     * @class: LightDataHost
     * @brief The LightDataHost struct is used for storing host side light data
     */
    struct LightDataHost
    {
        cl_int id;
        cl_int type;
        cl_float4 color;
        cl_float3 function;
        cl_float4 pos;
        cl_float4 dir;

        cl_float radius;
        cl_float penumbra;
        cl_float angle;
        cl_float width;
        cl_float height;
    };

    /**
     * @class: ObjectDataHost
     * @brief The ObjectDataHost struct is used for storing host side single object
     */
    struct ObjectDataHost
    {
        cl_float16 transform;
        cl_float16 invTransform;
        cl_float16 invTWithoutTrans;
        cl_int type;
        // Only support general texture. Bump texture not supported
        cl_int texHandle;
        cl_float texBlend;
        cl_float2 texRepeat;
        cl_int texIsUsed;
        cl_int texMapID;
        cl_int texWidth;
        cl_int texHeight;
        // Materials
        cl_float4 diffuse;
        cl_float4 ambient;
        cl_float4 reflective;
        cl_float4 specular;
        cl_float4 transparent;
        cl_float4 emmisive;
        cl_float shininess;
        cl_float ior;
    };

    /**
     * @class: GlobalSettingHost
     * @brief The GlobalSettingHost struct is used for holding host side
     *        global setting
     */
    struct GlobalSettingHost
    {
        cl_int useShadow;
        cl_int usePointLight;
        cl_int useDirectionalLight;
        cl_int useSpotLight;
        cl_int showTexture;
        cl_int useSupersampling;
        cl_int traceNum;
        cl_int useReflection;
        cl_int useKdTree;
    };

    GPURayScene();
    GPURayScene(CLPack* cl,
                OrbitCamera* camera,
                Scene* scene,
                GLuint screenTexHandle,
                GLuint screenPbo,
                GLuint screenWidth,
                GLuint screenHeight);
    ~GPURayScene();

    /**
     * @brief render: render loop
     */
    void render();

    /**
     * @brief syncGlobalSettings: copy the current global setting to CL structure
     */
    void syncGlobalSettings();


private:

    /**
     * @brief pushSceneData: push scene data into openCL structure
     * @param scene: the pointer to the scene
     */
    void pushSceneData(Scene* scene);

    /**
     * @brief setKernelArgs: set kernel arguments
     */
    void setKernelArgs();

    /**
     * @brief displayScreenTex: display screen texture
     */
    void displayScreenTex();

    /**
     * @brief updateScreenTexFromPBO: update screen texture from pixel buffer
     */
    void updateScreenTexFromPBO();

    /**
     * @brief copySceneColor: copy CS123SceneColor to cl_float4
     * @param color: color
     * @return: color in cl_float4
     */
    cl_float4 copyColor(CS123SceneColor color);

    /**
     * @brief copySceneMatrix: copy Matrix4x4 into cl_float16
     * @param mat: matrix
     * @return: matrix in cl_float16
     */
    cl_float16 copyMatrix(Matrix4x4 mat);

    /**
     * @brief copySceneVector4
     * @param v: vector4
     * @return: result in cl_float4 format
     */
    cl_float4 copyVector4(Vector4 v);

    /**
     * @brief copySceneVector3: copy Vector3 into cl_float3
     * @param v: vector3
     * @return: result in cl_float3 format
     */
    cl_float3 copyVector3(Vector3 v);

    /**
     * @brief copyKdTree: copy the kdtree into host side structure
     * @param tree: the CPU side tree
     */
    void copyKdTree(KdTree* tree);

    /**
     * @brief dumpCLKdTree: dump host side kd tree into file (just for debugging)
     * @param fileName: file name
     */
    void dumpCLKdTree(std::string fileName);

    /**
     * @brief dumpCLKdTreeInfoLeaf: dump host side kdtree node into output stream
     * @param curIndex: current index
     * @param ofile: output stream
     * @param depth: the depth of the node
     */
    void dumpCLKdTreeInfoLeaf(int curIndex,  std::ofstream& ofile, int depth);

    /**
     * @brief initCLBuffers: initilize openCL buffers
     */
    void initCLBuffers();

    OrbitCamera* m_camera; // Orbit camera
    CLPack* m_cl; // Opencl Package (general stuff: work size, platform id, etc)
    cl_float4 m_global; // Global data
    QVector<LightDataHost> m_lightData; // Host side light data in the scene
    QVector<ObjectDataHost> m_objects; // Host side object list in the scene

    QVector<KdTreeNodeHost> m_kdNodes; // Host side kd tree nodes
    QVector<ObjectNodeHost> m_objNodes; // Host side object nodes

    GLuint m_screenTex; // Screen texture handle
    GLuint m_screenPbo; // Screen pixel buffer handle

    GLuint m_screenWidth; // Screen width
    GLuint m_screenHeight; // Screen height

    GlobalSettingHost m_globalSetting; // Host side global setting

    cl_mem m_cmGlobal; // CL buffer for global setting
    cl_mem m_cmLight; // CL buffer for light data
    cl_mem m_cmObject; // CL buffer for object list

    cl_mem m_cmKdNodes; // CL buffer for kdtree nodes
    cl_mem m_cmObjNodes; // CL buffer for object nodes

    cl_mem m_cmTexPixelBuffer; // CL buffer for pixel buffer
    cl_mem m_cmOffsetBuffer; // CL buffer for offset

    QVector<cl_uint> m_textureOffsets; // CL buffer for textures' offset

    cl_int m_pixelNum; // CL buffer for pixel number
    cl_uint* m_pixels; // CL buffer for all pixels (texture)

    QVector<cl_GLuint> m_textureHandles; // CL buffer for texture handles
};

#endif // GPURayScene_H
