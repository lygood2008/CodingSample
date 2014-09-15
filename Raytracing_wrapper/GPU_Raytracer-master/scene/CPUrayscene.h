/*!
    @file CPURayScene.h
    @desc: Scene holder for CPU rendering
    @author: yanli
    @date: May 2013
 */

#ifndef CPURayScene_H
#define CPURayScene_H

#include "scene.h"
#include "aabb.h"

#define THREAD_NUM 8 // Thread number

class View2D;
class OrbitCamera;
class KdTree;
class Scene;
/**
 * @class: CPURayScene
 * @brief The CPURayScene class is used for tracing the scene using CPU
 */
class CPURayScene
{
public:

    CPURayScene();
    CPURayScene(Scene* scene);

    ~CPURayScene();

    /**
     * @brief traceScene: do ray tracing on the scene
     * @param view2D: pointer to the view structure
     * @param camera: pointer to the orbit camera
     * @param width: width of the canvas
     * @param height: height of the canvas
     */
    void traceScene(View2D* view2D,
                    OrbitCamera* camera,
                    int width,
                    int height);

protected:

    CS123SceneGlobalData m_globalData; // Scene global data
    QList<CS123SceneLightData> m_lightData; // Light data
    QVector<SceneObject> m_objects; // Object list
    KdTree* m_tree; // Pointer to the kdtree
    AABB m_extends; // Bounding box for the whole scene
};

#endif // CPURayScene_H
