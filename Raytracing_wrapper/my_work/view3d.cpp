/*!
    @file View3D.cpp
    @desc: definitions of View3D class
    @author: yanli
    @date: May 2013
 */

#include <QApplication>
#include <QKeyEvent>
#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QMouseEvent>

#include <sstream>
#include <iostream>

#include "view3d.h"
#include "scene.h"
#include "GPUrayscene.h"
#include "CS123XmlSceneParser.h"

#include "shape_draw.h"
#include "utils.h"
#include "resource_loader.h"

#include <GL/glx.h>
#include "oclUtils.h"
#include "clDumpGPUInfo.h"

using std::endl;
using std::cout;
using std::cerr;
using std::stringstream;

extern "C"
{
    void glBindBuffer(GLenum target, GLuint buffer);
    void glGenBuffers(GLsizei n, GLuint *buffers);
    void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data,
                      GLenum usage);
    void glBufferSubData(GLenum target,GLintptr offset, GLsizeiptr size,
                         const GLvoid * data);
    void glGetBufferParameteriv(GLenum target, GLenum value, GLint * data);
    void glDeleteBuffers(GLsizei n, const GLuint* buffers);
}

/* static variables */
static const char* clRayTraceGPU = "./OpenCL/shader/raytraceGPU.cl";
static const char* fontBMP       = "./resource/Font.bmp";

View3D::View3D(QWidget *parent) : QGLWidget(parent),
    m_timer(this), m_prevTime(0), m_prevFps(0.f), m_fps(0.f)
{
    this->context();
    // View3D needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // View3D needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The game loop is implemented using a timer
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));

    m_clock.start();

    m_mouseLeftDown   = false;
    m_mouseRightDown  = false;
    m_mouseMiddleDown = false;
    m_curShape        = 0;
    m_supportGPU      = false;
    m_gpuActivated    = false;
    m_scene           = NULL;
    m_quad            = NULL;
    m_gpuscene        = NULL;

    // fix the size of the window
    setFixedSize(WIN_WIDTH,WIN_HEIGHT);

    m_camera.setRatio((float)WIN_WIDTH / (float)WIN_HEIGHT);
}

View3D::~View3D()
{
    if (m_scene)
    {
        delete m_scene;
    }
    if (m_gpuscene)
    {
        delete m_gpuscene;
    }

    if (m_screenTex != 0)
        glDeleteTextures(1, &m_screenTex);

    if (m_quad)
        gluDeleteQuadric(m_quad);

    releaseShapes();
    deletePBO(&m_pbo);
    releaseCL();
    killFont();
}

Scene* View3D::loadScene(const QString& sceneName)
{
    if (!sceneName.isNull())
    {
        CS123XmlSceneParser parser(qPrintable(sceneName));
        if (parser.parse())
        {
            std::cerr << "Scene " << qPrintable(sceneName) << std::endl;
            // Delete the old scene
            if (m_scene)
            {
                delete m_scene;
            }

            if (m_gpuscene)
                delete m_gpuscene;

            Scene* newScene = new Scene;
            Scene::parse(newScene, &parser);

            // Set the camera for the new scene
            CS123SceneCameraData camera_data;
            if (parser.getCameraData(camera_data))
            {
                camera_data.pos.data[3]  = 1;
                camera_data.look.data[3] = 0;
                camera_data.up.data[3]   = 0;

                // We get rid of the camera data because we always use
                // Orbit camera
                cout << " We get rid of the camera data in the scene file"
                     << " Because we always use orbit camera" << endl;
            }

            m_scene = newScene;

            if (settings.useKdTree)
                m_scene->buildKdTree();

            return m_scene;
        }
        else
        {
            QMessageBox::critical(this, "Error", "Could not load scene \"" +
                                  sceneName + "\"");
            return NULL;
        }
    }
    return NULL;
}

void View3D::createGPUScene()
{
    if (m_scene)
        m_gpuscene = new GPURayScene(&m_cl,
                                     &m_camera,
                                     m_scene,
                                     m_screenTex,
                                     m_pbo,
                                     WIN_WIDTH,
                                     WIN_HEIGHT);
}

void View3D::initializeGL()
{
    // All OpenGL initialization *MUST* be done during or after this
    // method. Before this method is called, there is no active OpenGL
    // context and all OpenGL calls have no effect.

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DITHER);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_MULTISAMPLE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Best perspective correction
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_LIGHTING);
    // Light's color
    GLfloat ambientColor[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat diffuseColor[] = { 1.0f, 0.0f, 1.0, 1.0f };
    GLfloat lightPosition[] = {0, 0, 0, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);
    m_timer.start(1000 / 60);

    // Center the mouse, which is explained more in mouseMoveEvent() below.
    // This needs to be done here because the mouse may be initially outside
    // the fullscreen window and will not automatically receive mouse move
    // events. This occurs if there are two monitors and the mouse is on the
    // secondary monitor.
    QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));

    m_camera.updateMatrices();
    m_camera.update(WIN_WIDTH, WIN_HEIGHT);

    // Dump extensions
#ifdef DUMP_EXTENSION_INFO

    char* extensions = strdup((char*)glGetString(GL_EXTENSIONS));
    int len = strlen(extensions);
    // Separate it by new line instead of space
    for(int i = 0; i < len; i++)
    {
        if (extensions[i] == ' ')
            extensions[i] = '\n';
    }

    cout << "Extensions supported:" << endl <<  extensions << endl;

    if (isInString(extensions, "GL_ARB_multitexture"))
    {
        cout << "Multi-texture supported" << endl;
    }
    free(extensions);

#endif

    //Initialize resources
    initResources();

    // Init CL settings, should be done after init resources
    initCL();
}

void View3D::paintGL()
{
    if (settings.drawWireFrame || settings.drawNormals)
        glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    else
        glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera.update();

    if (m_scene && !m_gpuActivated)
    {
        if (settings.showAxis)
        {
            drawAxis();
        }
        renderScene();
    }
    else
    {
        GPUrender();
    }

    paintText();
}

void View3D::initCL()
{

    // We firstly dump our gpu info
    dumpGPUInfo();

    cl_int ciErrNum;
    // Get the platform
    ciErrNum = oclGetPlatformID(&m_cl.m_platform);
    if (ciErrNum != CL_SUCCESS)
    {
        cerr << " Get platform ID failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;

        m_supportGPU = false;
        return;
    }
    // Get th enumber of GPU devices available to the platform
    ciErrNum = clGetDeviceIDs(m_cl.m_platform,
                              CL_DEVICE_TYPE_GPU,
                              0,
                              NULL,
                              &m_cl.m_uiDevCount);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr << " Get device ids  failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        m_supportGPU = false;
        return;
    }
    else
    {
        cout << "There are " << m_cl.m_uiDevCount << " devices" << endl;
    }

    if (m_cl.m_uiDevCount == 0)
    {
        cerr << "No device available in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        m_supportGPU = false;
        return;
    }
    else
    {
        m_cl.m_deviceList.resize(m_cl.m_uiDevCount);
        ciErrNum = clGetDeviceIDs(m_cl.m_platform,
                                  CL_DEVICE_TYPE_GPU,
                                  m_cl.m_deviceList.size(),
                                  m_cl.m_deviceList.data(),
                                  NULL);
        if (ciErrNum != CL_SUCCESS)
        {
            cerr << "Get device ids failed in line:" << __LINE__ << ", File:"
                 << __FILE__ << endl;
            m_supportGPU = false;
            return ;
        }
        cl_context_properties props[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
            CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)m_cl.m_platform,
            0};

        cl_device_id device;
        for(int i = 0; i < m_cl.m_deviceList.size(); i++)
        {
            device = m_cl.m_deviceList.at(i);
            m_cl.m_context = clCreateContext(props,
                                             CL_DEVICE_TYPE_DEFAULT,
                                             &device,
                                             NULL,
                                             NULL,
                                             &ciErrNum);
            if (ciErrNum == CL_SUCCESS)
            {
                break;
            }
        }

        if (ciErrNum != CL_SUCCESS || m_cl.m_context == NULL)
        {
            cerr << "Get device context failed in line:" << __LINE__ <<", File:"
                 << __FILE__<< endl;
            m_supportGPU = false;
            return ;
        }
        else
        {
            m_cl.m_device = device;
            // Dump gpu info
            cout << "We are going to use:" << endl;
            cout << getPlatformName(m_cl.m_platform).toStdString().c_str()
                 << ": " << getDeviceName(device).toStdString().c_str() << endl;
        }
    }

    m_cl.m_queue = clCreateCommandQueue(m_cl.m_context, m_cl.m_device, 0,
                                        &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Create command queue failed in line:" << __LINE__ << ", File:"
             << __FILE__ << endl;
        m_supportGPU = false;
        return;
    }

    m_supportGPU = true;

    initCLProgramAndKernel();
}

void View3D::releaseCL()
{
    if (m_cl.m_queue)
    {
        clFinish(m_cl.m_queue);
        clReleaseCommandQueue(m_cl.m_queue);
    }
    if (m_cl.m_program)
        clReleaseProgram(m_cl.m_program);

    if (m_cl.m_context)
        clReleaseContext(m_cl.m_context);
    if (m_cl.m_kernelRay)
        clReleaseKernel(m_cl.m_kernelRay);

    if (m_cl.m_cmPbo)
        clReleaseMemObject(m_cl.m_cmPbo);
}

void View3D::initCLProgramAndKernel()
{
    cl_int ciErrNum;

    size_t szProgramLength;

    char* cSourceCL = oclLoadProgSource(clRayTraceGPU, "", &szProgramLength);

    m_cl.m_program = clCreateProgramWithSource(m_cl.m_context,
                                               1,
                                               (const char**)&cSourceCL,
                                               &szProgramLength,
                                               &ciErrNum);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Create program failed in line:"
             << __LINE__ << ", File:" << __FILE__ << endl;
        free(cSourceCL);
        return ;
    }
    ciErrNum = clBuildProgram(m_cl.m_program,
                              1,
                              &m_cl.m_device,
                              NULL,
                              NULL,
                              NULL);

    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Build program failed in line:"
             << __LINE__ << ", File:" << __FILE__ << endl;

        cerr << "The log is: " << endl;

        char* buildLog;
        size_t logSize;
        clGetProgramBuildInfo(m_cl.m_program, m_cl.m_device,
                              CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        buildLog = new char[logSize + 1];

        clGetProgramBuildInfo(m_cl.m_program, m_cl.m_device,
                              CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
        buildLog[logSize + 1] = '\0';
        cout <<  buildLog  <<   endl;
        delete []buildLog;

        free(cSourceCL);
        return;
    }

    m_cl.m_cmPbo = clCreateFromGLBuffer(m_cl.m_context, CL_MEM_WRITE_ONLY,
                                        m_pbo, &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        cerr << "Create buffer from GL failed in line:"
             << __LINE__ << ", File:" << __FILE__ << endl;
        free(cSourceCL);
        return ;
    }

    // Create the kernel and setup the arguments

    m_cl.m_kernelRay = clCreateKernel(m_cl.m_program, "raytrace", &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
            cerr << "Create kernel failed in line:"
                 << __LINE__ << ", File:" << __FILE__ << endl;
            free(cSourceCL);
            return ;
     }

    m_cl.m_localWorkSize[0]  = GPU_LOCAL_WORK_SIZE_X;
    m_cl.m_localWorkSize[1]  = GPU_LOCAL_WORK_SIZE_Y;
    m_cl.m_globalWorkSize[0] = shrRoundUp(m_cl.m_localWorkSize[0], WIN_WIDTH);
    m_cl.m_globalWorkSize[1] = shrRoundUp(m_cl.m_localWorkSize[1], WIN_HEIGHT);

    cout << "Local work size" << m_cl.m_localWorkSize[0] << " "
         << m_cl.m_localWorkSize[1] << endl;
    cout << "Global work size" << m_cl.m_globalWorkSize[0] << " "
         << m_cl.m_globalWorkSize[1] << endl;

    cout << "Max work group item size is " << CL_DEVICE_MAX_WORK_ITEM_SIZES;

    if (m_cl.m_localWorkSize[0] * m_cl.m_localWorkSize[1] >
            CL_DEVICE_MAX_WORK_ITEM_SIZES)
    {
        cerr << "Invalid local work size" << endl;
        free(cSourceCL);
        return;
    }
    free(cSourceCL);
}

void View3D::renderScene()
{
    if (m_scene)
        m_scene->render(this);
}

void View3D::resizeGL(int width, int height)
{
    m_camera.setRatio((float)width / (float)height);
    glViewport(0, 0, width, height);
}
/**
    when press the mouse, the force should be added to the velocity field
 **/
void View3D::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        m_camera.mouseDown(event->x(),event->y());
        m_mouseRightDown = true;
    }
    else if (event->button() == Qt::LeftButton)
    {
        m_mouseLeftDown = true;
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_camera.mouseDown(event->x(),event->y());
        m_mouseMiddleDown = true;
    }
}

void View3D::mouseMoveEvent(QMouseEvent *event)
{
    int deltaX = event->x() - width() / 2;
    int deltaY = event->y() - height() / 2;
    if (!deltaX && !deltaY) return;

    if (m_mouseRightDown)
    {
        m_camera.mouseMove(event->x(), event->y());
        m_camera.updateMatrices();
        m_camera.update();
    }
    else if (m_mouseMiddleDown)
    {
        m_camera.mouseMovePan(event->x(), event->y());
        m_camera.updateMatrices();
        m_camera.update();
    }
}

void View3D::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseLeftDown   = false;
    m_mouseRightDown  = false;
    m_mouseMiddleDown = false;
}

void View3D::wheelEvent(QWheelEvent *event)
{
    m_camera.mouseWheel(event->delta());
    m_camera.updateMatrices();
    m_camera.update();

}

void View3D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        QApplication::quit();
    }
    else if (event->key() == Qt::Key_L)
    {
        glPolygonMode(GL_FRONT, GL_LINE);
    }
    else if (event->key() == Qt::Key_P)
    {
        glPolygonMode(GL_FRONT, GL_POINT);
    }
    else if (event->key() == Qt::Key_W)
    {
        settings.drawWireFrame = !settings.drawWireFrame;
        glPolygonMode(GL_FRONT, GL_FILL);
    }
    else if (event->key() == Qt::Key_N)
    {
        settings.drawNormals = !settings.drawNormals;
    }
    else if (event->key() == Qt::Key_T)
    {
        settings.showTexture = !settings.showTexture;
        if (m_gpuscene)
            m_gpuscene->syncGlobalSettings();
    }
    else if (event->key() == Qt::Key_A)
    {
        settings.showAxis = !settings.showAxis;
    }
    else if (event->key() == Qt::Key_R)
    {
        settings.useReflection = !settings.useReflection;
        if (m_gpuscene)
            m_gpuscene->syncGlobalSettings();
    }
    else if (event->key() == Qt::Key_B)
    {
        settings.showBoundingBox = !settings.showBoundingBox;
    }
    else if (event->key() == Qt::Key_K)
    {
        settings.showKdTree = !settings.showKdTree;
    }
}

void View3D::paintText()
{
    glColor3f(1.f, 1.f, 1.f);

    // Combine the previous and current frame rate
    if (m_fps >= 0 && m_fps < 1000)
    {
       m_prevFps *= 0.95f;
       m_prevFps += m_fps * 0.05f;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    QString str;
    str = "FPS: " + QString::number((int)(m_prevFps));
    printText(10, WIN_HEIGHT - 20, str.toStdString().c_str() , 0);

    str = "Delta: " + QString::number((float)(m_delta));
    printText(10, WIN_HEIGHT - 35,  str.toStdString().c_str(), 0);

    stringstream ss;
    ss << "A: Show axis = " << (settings.showAxis ? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT -  50,  str.toStdString().c_str(), 0);
    ss.str("");

    ss << "W: Wireframe mode = " << (settings.drawWireFrame ? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT -  65,  str.toStdString().c_str(), 0);
    ss.str("");

    ss << "N: Show normal = " << (settings.drawNormals ? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT - 80,  str.toStdString().c_str(), 0);
    ss.str("");

    ss << "T: Show texture = " << (settings.showTexture ? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT -  95,  str.toStdString().c_str(), 0);
    ss.str("");

    ss << "B: Show bounding box = "
       << (settings.showBoundingBox? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT -  110,  str.toStdString().c_str(), 0);
    ss.str("");

    ss << "K: Show kdTree = " << (settings.showKdTree? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT -  125,  str.toStdString().c_str(), 0);
    ss.str("");

    ss << "R: Use recursive ray tracing = "
       << (settings.useReflection ? "true" : "false");
    str = ss.str().c_str();
    printText(10, WIN_HEIGHT -  140,  str.toStdString().c_str(), 0);
    ss.str("");

    glDisable(GL_BLEND);
}

void View3D::updateTime()
{
    long time  = m_clock.elapsed();
    m_delta    = (time - m_prevTime) / 1000.f;
    m_fps      = 1000.f /  (time - m_prevTime);
    m_prevTime = time;
}

void View3D::drawAxis()
{

    glColor3f(0, 0, 1);
    gluCylinder(m_quad, 0.1, 0.1, 3, 10, 10); // Z
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glColor3f(1, 0, 0);
    gluCylinder(m_quad, 0.1, 0.1, 3, 10, 10); // X
    glPopMatrix();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glColor3f(0, 1, 0);
    gluCylinder(m_quad, 0.1, 0.1, 3, 10, 10); // Y
    glPopMatrix();
    PRINT_GL_ERROR();
}

void View3D::initializeShapes()
{
    buildCubeVBO(2, m_vbos.cubeVBO, m_vbos.cubeElementVBO);
    buildCylinderVBO(24, m_vbos.cylinderVBO, m_vbos.cylinderElementVBO);
    buildConeVBO(24, m_vbos.coneVBO, m_vbos.coneElementVBO);
    buildSphereVBO(24, 24, m_vbos.sphereVBO, m_vbos.sphereElementVBO);

    PRINT_GL_ERROR();

    if (m_vbos.cubeVBO == 0 || m_vbos.cubeElementVBO == 0)
        cerr << "The cube VBO is not initliazed correctly" << endl;
    if (m_vbos.cylinderVBO == 0 || m_vbos.cylinderElementVBO == 0)
        cerr << "The cylinder VBO is not initliazed correctly" << endl;
    if (m_vbos.coneVBO == 0 || m_vbos.coneElementVBO == 0)
        cerr << "The cone VBO is not initliazed correctly" << endl;
    if (m_vbos.sphereVBO == 0 || m_vbos.sphereElementVBO == 0)
        cerr << "The sphere VBO is not initliazed correctly" << endl;
}

void View3D::releaseShapes()
{
    if (m_vbos.cubeVBO)
        glDeleteBuffers(1, &m_vbos.cubeVBO);

    if (m_vbos.cylinderVBO)
        glDeleteBuffers(1, &m_vbos.cylinderVBO);

    if (m_vbos.coneVBO)
        glDeleteBuffers(1, &m_vbos.coneVBO);

    if (m_vbos.sphereVBO)
        glDeleteBuffers(1, &m_vbos.sphereVBO);

    if (m_vbos.cubeElementVBO)
        glDeleteBuffers(1, &m_vbos.cubeElementVBO);

    if (m_vbos.cylinderElementVBO)
        glDeleteBuffers(1, &m_vbos.cylinderElementVBO);

    if (m_vbos.coneElementVBO)
        glDeleteBuffers(1, &m_vbos.coneElementVBO);

    if (m_vbos.sphereElementVBO)
        glDeleteBuffers(1, &m_vbos.sphereElementVBO);
}

void View3D::initResources()
{
    initializeShapes();

    createPBO(&m_pbo, WIN_WIDTH, WIN_HEIGHT);
    createTexture(&m_screenTex, WIN_WIDTH, WIN_HEIGHT);

    m_quad = gluNewQuadric();

    loadBMPTexture(&m_fontHandle, fontBMP);
    genFontDisplayList();
}

void View3D::drawShape(SHAPE_TYPE type, bool drawNorms)
{
    switch(type)
    {
    case CUBE:
        if (drawNorms)
        {
            drawNormals(m_vbos.cubeVBO, (cubeTess + 1) * (cubeTess + 1) * 6);
        }
        else
        {
            drawCube(m_vbos.cubeVBO, m_vbos.cubeElementVBO, m_dumTex);
        }
        break;
    case CYLINDER:
        if (drawNorms)
        {
            drawNormals(m_vbos.cylinderVBO, (cylinderTess + 1) * 4);
        }
        else
        {
            drawCylinder(m_vbos.cylinderVBO, m_vbos.cylinderElementVBO, m_dumTex);
        }
        break;
    case SPHERE:
        if (drawNorms)
        {
            drawNormals(m_vbos.sphereVBO, (sphereTess[1] + 1) * (sphereTess[0] + 1));
        }
        else
        {
            drawSphere(m_vbos.sphereVBO, m_vbos.sphereElementVBO, m_dumTex);
        }
        break;
    case CONE:
        if (drawNorms)
        {
            drawNormals(m_vbos.coneVBO, (coneTess + 1) * 3);
        }
        else
        {
            drawCone(m_vbos.coneVBO, m_vbos.coneElementVBO, m_dumTex);
        }
        break;
    default:
        cerr << "[drawShape] Invalid type" << endl;
        assert(0);
        break;
    }
}

void View3D::initBuffer()
{
    createPBO(&m_pbo, WIN_WIDTH, WIN_HEIGHT);
}

void View3D::createPBO(GLuint* pbo, const int imageWidth, const int imageHeight)
{
    // set up data parameter
    int numTexels   = imageWidth * imageHeight;
    int numValues   = numTexels * 4;
    int sizeTexData = sizeof(GLubyte) * numValues;

    glGenBuffers(1, pbo);
    glBindBuffer(GL_ARRAY_BUFFER, *pbo);

    glBufferData(GL_ARRAY_BUFFER, sizeTexData, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void View3D::deletePBO(GLuint* pbo)
{
    if (*pbo != 0)
    {
        glDeleteBuffers(1, pbo);
    }
}

void View3D::GPUrender()
{
    if (!m_supportGPU)
    {
        cerr << "GPU render is not supported" << endl;
        return;
    }

    if (!m_cl.m_kernelRay)
    {
        cerr << "Kernel has not been set up" << endl;
        return;
    }
    if (m_gpuscene && m_gpuActivated)
        m_gpuscene->render();
}

void View3D::genFontDisplayList()
{
    float cx, cy;         /* the character coordinates in our texture */
    m_fontBase = glGenLists(256);
    glBindTexture(GL_TEXTURE_2D, m_fontHandle);
    for (int loop = 0; loop < 256; loop++)
    {
        cx = (float) (loop % 16) / 16.0f;
        cy = (float) (loop / 16) / 16.0f;

        glNewList(m_fontBase + loop, GL_COMPILE);
        glBegin(GL_QUADS);
        glTexCoord2f(cx, 1 - cy - 0.0625f);
        glVertex2i(0, 0);
        glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);
        glVertex2i(16, 0);
        glTexCoord2f(cx + 0.0625f, 1 - cy);
        glVertex2i(16, 16);
        glTexCoord2f(cx, 1 - cy);
        glVertex2i(0, 16);
        glEnd();
        glTranslated(10, 0, 0);
        glEndList();
    }
}

void View3D::killFont()
{
    glDeleteLists(m_fontBase, 256);
}

void View3D::printText(int x, int y, const char* string, int set)
{
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, m_fontHandle);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslated(x, y, 0);
    glListBase(m_fontBase - 32 + (128 * set));
    glCallLists(strlen(string), GL_BYTE, string);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

/**
    tick() will be called each frame
  **/
void View3D::tick()
{
    updateTime();
    update();
}


