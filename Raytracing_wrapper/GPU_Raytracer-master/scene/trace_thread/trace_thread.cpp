/*!
    @file trace_thread.cpp
    @desc: definitions of TraceThread class
    @author: yanli
    @date: May 2013
 */

#include "trace_thread.h"

TraceThread::TraceThread(QObject *parent) : QThread(parent)
{

}

void TraceThread::pack(BGRA* pixel,
                       const int width,
                       const int height,
                       const int beginIndex,
                       const int endIndex,
                       CS123SceneGlobalData& global,
                       QVector<SceneObject>& objects,
                       QList<CS123SceneLightData>& lights,
                       Vector4& eyePos,
                       float near,
                       Matrix4x4& invViewTransMat,
                       KdTree* tree,
                       AABB extends)
{

    m_pixel           = pixel;
    m_width           = width;
    m_height          = height;
    m_beginIndex      = beginIndex;
    m_endIndex        = endIndex;
    m_global          = global;
    m_objects         = objects;
    m_lights          = lights;
    m_eyePos          = eyePos;
    m_near            = near;
    m_invViewTransMat = invViewTransMat;
    m_tree            = tree;
    m_extends         = extends;
}

TraceThread::TraceThread(BGRA *pixel,
                         const int width,
                         const int height,
                         const int beginIndex,
                         const int endIndex,
                         CS123SceneGlobalData &global,
                         QVector<SceneObject> &objects,
                         QList<CS123SceneLightData> &lights,
                         vec4<REAL> &eyePos,
                         float near,
                         Matrix4x4 &invViewTransMat,
                         KdTree* tree,
                         AABB extends)
{

    m_pixel           = pixel;
    m_width           = width;
    m_height          = height;
    m_beginIndex      = beginIndex;
    m_endIndex        = endIndex;
    m_global          = global;
    m_objects         = objects;
    m_lights          = lights;
    m_eyePos          = eyePos;
    m_near            = near;
    m_invViewTransMat = invViewTransMat;
    m_tree            = tree;
    m_extends         = extends;
}

TraceThread::~TraceThread()
{

}

void TraceThread::render()
{

    doRayTrace(m_pixel,
               m_width,
               m_height,
               m_beginIndex,
               m_endIndex,
               m_global,
               m_objects,
               m_lights,
               m_eyePos,
               m_near,
               m_invViewTransMat,
               m_tree,
               m_extends
               );
}

void TraceThread::run()
{

   render();
}
