# 
# CS123 Final Project Starter Code
# Adapted from starter code graciously provided by CS195-U: 3D Game Engines
#

QT += core gui opengl xml

TARGET = final
TEMPLATE = app

# If you add your own folders, add them to INCLUDEPATH and DEPENDPATH, e.g.
# INCLUDEPATH += folder1 folder2
# DEPENDPATH += folder1 folder2

INCLUDEPATH += lib \
    math \
    support \
    global \
    scene \
    scene/trace_thread \
    scene/kdtree \
    intersect \
    shape \
    OpenCL \
    aabb \

DEPENDPATH += lib \
    math \
    support \
    global \
    scene \
    scene/trace_thread \
    scene/kdtree \
    intersect \
    shape \
    OpenCL \
    aabb \

SOURCES += support/main.cpp \
    support/mainwindow.cpp \
    support/camera.cpp \
    lib/glm.cpp \
    lib/targa.cpp \
    math/CS123Matrix.cpp \
    math/CS123Matrix.inl \
    math/CS123Vector.inl \
    support/view2d.cpp \
    support/view3d.cpp \
    scene/CS123XmlSceneParser.cpp \
    scene/scene.cpp \
    lib/utils.cpp \
    lib/recourceloader.cpp \
    scene/CPUrayscene.cpp \
    shape/shape_draw.cpp \
    intersect/cone_intersect.cpp \
    intersect/cube_intersect.cpp \
    intersect/cylinder_intersect.cpp \
    intersect/sphere_intersect.cpp \
    intersect/plane_intersect.cpp \
    scene/trace.cpp \
    intersect/intersect.cpp \
    scene/trace_thread/trace_thread.cpp \
    scene/GPUrayscene.cpp \
    OpenCL/oclUtils.cpp \
    OpenCL/clDumpGPUInfo.cpp \
    intersect/pos_check.cpp \
    aabb/aabb.cpp \
    scene/kdtree/kdtree.cpp \
    scene/kdtree/kdtreenode.cpp \
    intersect/kdbox_intersect.cpp \
    global/global.cpp

HEADERS += support/mainwindow.h \
    support/camera.h \
    lib/glm.h \
    lib/targa.h \
    global/global.h \
    math/CS123Algebra.h \
    global/CS123Common.h \
    math/vector.h \
    support/view2d.h \
    support/view3d.h \
    scene/CS123XmlSceneParser.h \
    scene/CS123SceneData.h \
    scene/CS123ISceneParser.h \
    scene/scene.h \
    lib/utils.h \
    scene/CPUrayscene.h \
    lib/resource_loader.h \
    shape/shape_draw.h \
    intersect/cone_intersect.h \
    intersect/cube_intersect.h \
    intersect/cylinder_intersect.h \
    intersect/sphere_intersect.h \
    intersect/plane_intersect.h \
    intersect/intersect.h \
    scene/trace.h \
    scene/trace_thread/trace_thread.h \
    scene/GPUrayscene.h \
    OpenCL/CL/opencl.h \
    OpenCL/CL/cl_platform.h \
    OpenCL/CL/cl_gl_ext.h \
    OpenCL/CL/cl_gl.h \
    OpenCL/CL/cl_ext.h \
    OpenCL/CL/cl.h \
    OpenCL/oclUtils.h \
    OpenCL/shrUtils.h \
    OpenCL/clDumpGPUInfo.h \
    intersect/pos_check.h \
    aabb/aabb.h \
    scene/kdtree/kdtree.h \
    scene/kdtree/kdtreenode.h \
    scene/kdtree/kdtreecommon.h \
    intersect/kdbox_intersect.h \
    ui_mainwindow.h

OTHER_FILES += \
    OpenCL/shader/raytraceGPU.cl
FORMS += \
    mainwindow.ui

unix|win32: LIBS += -lGLU -lglut -lOpenCL -L/home/genxfsim/GPU_Raytracer-master/OpenCL/lib -loclUtil_i386 -lshrutil_i386
