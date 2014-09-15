/*!
    @file View3D.h
    @desc: declarations of View3D class
    @author: yanli
    @date: May 2013
 */

#ifndef VIEW3D_H
#define VIEW3D_H


#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <QHash>
#include <QVector>
#include <QString>

#include "global.h"
#include "camera.h"
#include "CL/cl.h"
#include "GL/glu.h"

class Scene;
class Cube;
class Cone;
class GPURayScene;

/**
 * @struct: VnoHandles
 * @brief The VboHandles struct is merely used for storing vbo handles
 */
struct VboHandles
{
    GLuint cubeVBO;
    GLuint cubeElementVBO;
    GLuint cylinderVBO;
    GLuint cylinderElementVBO;
    GLuint coneVBO;
    GLuint coneElementVBO;
    GLuint sphereVBO;
    GLuint sphereElementVBO;
};

/**
 * @struct: CLPack
 * @brief The CLPack struct is merely used for storing OpenCL platform informations
 */
struct CLPack
{
    cl_platform_id m_platform;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_program m_program;
    cl_uint m_uiDevCount;
    cl_device_id m_device;
    cl_kernel m_kernelRay;

    cl_mem m_cmPbo;
    cl_mem m_cmOut;

    size_t m_localWorkSize[2];
    size_t m_globalWorkSize[2];

    QVector<cl_device_id> m_deviceList;
};

/**
 * @class View3D
 * @brief The View3D class inherits QGLWidghet; its job is to render the scene
 *        using GL
 */
class View3D :
        public QGLWidget
{
    Q_OBJECT

public:

    // Shape Type
    enum SHAPE_TYPE
    {
        CUBE = 0,
        CYLINDER,
        SPHERE,
        CONE
    };

    View3D(QWidget *parent);
    ~View3D();

    /**
     * @brief loadScene: load a new scene from a path
     * @param sceneName: the scene's name
     * @return: the pointer to the scene
     */
    Scene* loadScene(const QString& sceneName);

    /**
     * @brief createGPUScene: basically create a m_gpuscene from m_scene
     */
    void createGPUScene();

    /**
     * Getters
     */
    OrbitCamera* getCamera(){ return &m_camera; }

    const VboHandles* getVBOs(){ return &m_vbos; }

    Scene* getScene() { return m_scene; }

    bool isActivatedGPUtrace(){ return m_gpuActivated; }

    /**
     * @brief activateGPUtrace: set m_gpuActivated to the given boolean
     * @param checked: boolean value
     */
    void activateGPUtrace(bool checked) { m_gpuActivated = checked; }

    /**
     * @brief syncGPUGlobalSetting: sync global settings to GPU scene
     */
    void syncGPUGlobalSetting()
    {
        if(m_gpuscene)
            m_gpuscene->syncGlobalSettings();
    }

private:

    /**
     * @brief initializeGL: initialize OpenGL
     */
    virtual void initializeGL();

    /**
     * @brief paintGL: function for doing painting
     */
    virtual void paintGL();

    /**
     * @brief initCL: initialize OpenCL
     */
    void initCL();

    /**
     * @brief releaseCL: release OpenCL
     */
    void releaseCL();

    /**
     * @brief initCLProgramAndKernel: initialize OpenCL settings
     */
    void initCLProgramAndKernel();

    /**
     * @brief renderScene: render the scene
     */
    void renderScene();

    /**
     * @brief resizeGL: resize canvas
     * @param width: new width
     * @param height: new height
     */
    virtual void resizeGL(int width, int height);

    /**
     * Mouse Event handlers
     */
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    /**
     * KeyEvent handlers
     */
    virtual void keyPressEvent(QKeyEvent *event);

    /**
     * @brief paintText: draw text
     */
    void paintText();

    /**
     * @brief updateTime: update current time
     */
    void updateTime();

    /**
     * @brief drawAxis: draw axis
     */
    void drawAxis();

    /**
     * @brief initializeShapes: initialize vbos for shapes
     */
    void initializeShapes();

    /**
     * @brief releaseShapes: release vbos for shapes
     */
    void releaseShapes();

    /**
     * @brief initResources: wrapper functions for initialize resources
     */
    void initResources();

    /**
     * @brief drawShape: draw shape
     * @param type: the shape type
     * @param drawNormals: draw normals or not
     */
    void drawShape(SHAPE_TYPE type, bool drawNormals);

    /**
     * @brief initBuffer: wrapper for doing initialization pixel buffer
     */
    void initBuffer();

    /**
     * @brief createPBO: create a pixel buffer
     * @param pbo: the pointer to the pixel buffer handle
     * @param imageWidth: image width
     * @param imageHeight: image height
     */
    void createPBO(GLuint* pbo, const int imageWidth, const int imageHeight);

    /**
     * @brief deletePBO: delete the pixel buffer
     * @param pbo: the pointer to the pixel buffer handle
     */
    void deletePBO(GLuint* pbo);

    /**
     * @brief GPUrender: render GPU scene (use GPU to do ray tracing)
     */
    void GPUrender();

    /**
     * @brief genFontDisplayList: generate font's display list
     */
    void genFontDisplayList();

    /**
     * @brief killFont: release display list
     */
    void killFont();

    /**
     * @brief printText: paint text on the screen
     * @param x: x position on the screen
     * @param y: y position on the screen
     * @param string: string
     * @param set: font set
     */
    void printText(int x, int y, const char* string, int set);

    QTimer m_timer; // Timer
    QTime m_clock; // Clock
    int m_prevTime; // Track previous time
    float m_prevFps; // Track previous fps
    float m_fps; // Current fps
    float m_delta; // Delta time between two continuous frame

    /**
     * Booleans, just for switch
     */
    bool m_mouseLeftDown;
    bool m_mouseRightDown;
    bool m_mouseMiddleDown;
    bool m_gpuActivated;
    bool mouse_move;
    bool m_useGPU;
    bool m_supportGPU;

    OrbitCamera m_camera; // My camera, we use orbit camera

    /**
     * Display list for font
     */
    GLuint m_fontBase;
    GLuint m_fontHandle;

    Scene* m_scene; // scene
    GPURayScene* m_gpuscene; // GPUrayscene
    VboHandles m_vbos; // VBO handles
    int m_curShape; // My current shape
    CLPack m_cl; // OpenCL information
    GLuint m_screenTex; // Screen texture, used for rendering
    GLuint m_dumTex; // Dummy texture, just used for texting
    GLuint m_pbo; // Pixel buffer handle
    unsigned int* m_bufImage; // Buffered image data
    GLUquadric* m_quad; // For drawing quadric

private slots:

    /**
     * @brief tick: callback function for timer
     */
    void tick();
};

#endif // View3D_H
