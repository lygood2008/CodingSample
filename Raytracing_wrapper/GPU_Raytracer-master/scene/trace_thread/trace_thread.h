/*!
    @file trace_thread.h
    @desc: declarations of TraceThread class
    @author: yanli
    @date: May 2013
 */

#ifndef TRACE_THREAD_H
#define TRACE_THREAD_H

#include "trace.h"
#include "scene.h"
#include <QHash>
#include <QThread>

class KdTree;

/**
 * @class: TraceThread
 * @brief The TraceThread class is the structure for running a separate thread
 *        and holding the necessary data for running
 */
class TraceThread :
        public QThread
{
    Q_OBJECT

public:

    TraceThread(QObject *parent = 0 );


    TraceThread(BGRA* pixel,
                const int width,
                const int height,
                const int beginIndex,
                const int endIndex,
                CS123SceneGlobalData& global,
                QVector<SceneObject>& objects,
                QList<CS123SceneLightData>& lights,
                Vector4& eyePos,
                const float near,
                Matrix4x4& invViewTransMat,
                KdTree* tree,
                AABB extends);

    /**
     * @brief pack: copy all of the necessary data into thread
     * @param pixel: the pixels
     * @param width: width
     * @param height: height
     * @param beginIndex: begin index of pixels
     * @param endIndex: end index of pixels
     * @param global: global scene data
     * @param objects: object list
     * @param lights: light list
     * @param eyePos: eye position
     * @param near: near plane
     * @param invViewTransMat: inverse view transformation matrix
     * @param tree: the pointer to the tree
     * @param extends: the bounding box of the whole scene
     */
    void pack(BGRA* pixel,
              const int width,
              const int height,
              const int beginIndex,
              const int endIndex,
              CS123SceneGlobalData& global,
              QVector<SceneObject>& objects,
              QList<CS123SceneLightData>& lights,
              Vector4& eyePos,
              const float near,
              Matrix4x4& invViewTransMat,
              KdTree* tree,
              AABB extends);

    /**
     * @brief render: do rendering
     */
    void render();


    ~TraceThread();

    /**
     * @brief run: start the thread
     */
    void run();

private:

    BGRA* m_pixel; // Pixel buffers
    int m_width; // Width of the canvas
    int m_height; // Height of the canvas
    int m_beginIndex; // Begin index of pixels
    int m_endIndex; // End index of pixels
    CS123SceneGlobalData m_global; // Global scene data
    QVector<SceneObject> m_objects; // Object lists
    QList<CS123SceneLightData> m_lights; // Lights
    Vector4 m_eyePos; // Eye position
    float m_near; // Near plane
    Matrix4x4 m_invViewTransMat; // Inverse view transformation matrix
    KdTree* m_tree; // Pointer to the kdtree
    AABB m_extends; // Bounding box for the whole scene
};

#endif
