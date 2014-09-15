/*!
    @file global.h
    @desc: declarations of the global variables
    @author: yanli
    @date: May 2013
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <qgl.h>
#include <iostream>
#include "GPUrayscene.h"
#include <QVector>

#define WIN_WIDTH 600 // Window width
#define WIN_HEIGHT 600 // Window height

#define GPU_LOCAL_WORK_SIZE_X 32 // GPU working size in X dimension
#define GPU_LOCAL_WORK_SIZE_Y 32 // GPU working size in Y dimension

#define MAX_ARRAY 1024 // Max array size

#define DUMP_EXTENSION_INFO // Flag for dump extension info

// Trace mode, GPU/CPU
enum TRACEMODE
{
    GPU,
    CPU
};

// Polygon display mode
enum POLYGONMODE
{
    POINT,
    LINE,
    FILL
};

/**
 * @struct: Setting
 * @brief The Settings struct is a structure holding global settings
 */
struct Settings
{
    Settings();
    void initSettings();

    bool useLighting;
    TRACEMODE traceMode;
    bool drawWireFrame;
    bool drawNormals;
    bool useDirectionalLights;
    bool usePointLights;
    bool showAxis;
    bool showTexture;
    bool useMultithread;
    bool useSupersampling;
    bool useShadow;
    bool useSpotLights;
    bool useReflection;
    bool showBoundingBox;
    bool showKdTree;
    bool useKdTree;

    int traceRaycursion;
    int traceThreadNum;
};

// External variables
extern Settings settings;
extern QVector<KdTreeNodeHost> kdnode_test;
extern QVector<ObjectNodeHost> objnode_test;

#define OFFSET(offset) ((char*)NULL + offset)

/**
 * @brief getGLErrorString: get GL error string
 * @return: the error string
 */
const char* getGLErrorString();

// Macro for printing GL errors
#define PRINT_GL_ERROR(void) ({ \
    const char* error = getGLErrorString();\
    if(error) \
    {\
        std::cout<<"GL error: "<<error<<std::endl;\
    }\
})
#endif // GLOBAL_H
