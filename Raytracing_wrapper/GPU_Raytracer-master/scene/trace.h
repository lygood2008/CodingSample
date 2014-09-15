/*!
    @file trace.h
    @desc: the declarations of wrapper functions for doing ray tracing
    @author: yanli
    @date: May 2013
 */

#ifndef TRACE_H
#define TRACE_H

#include "CS123SceneData.h"
#include "scene.h"

/**
 * @brief doRayTrace: do ray tracing, inner wrapper function.
 * @param data: pixels
 * @param width: wdith of canvas
 * @param height: height of canvas
 * @param beginIndex: beginIndex of pixels
 * @param endIndex: endIndex of pixels
 * @param global: global scene data
 * @param objects: object list
 * @param lights: light data
 * @param eyePos: eye position
 * @param near: near plane
 * @param invViewTransMat: inverse of view transformation matrix
 * @param tree: pointer to the kdtree
 * @param extends: the bounding box of the scene
 */
void doRayTrace(BGRA* data,
                const int width,
                const int height,
                const int beginIndex,
                const int endIndex,
                const CS123SceneGlobalData& global,
                QVector<SceneObject>& objects,
                const QList<CS123SceneLightData>& lights,
                const Vector4& eyePos,
                const float near,
                const Matrix4x4& invViewTransMat,
                KdTree* tree,
                AABB extends);

/**
 * @brief recursiveTrace: recursive function calls
 * @param pos: position or eye or next start point
 * @param d: direction vector
 * @param global: global scene data
 * @param objects: object list
 * @param lights: light data
 * @param tree: pointer to the tree
 * @param extends: bounding box of the scene
 * @param curIndex: current index of pixels
 * @param count: recursive depth count;
 * @return: result color
 */
CS123SceneColor recursiveTrace(const Vector4& pos,
                               const Vector4& d,
                               const CS123SceneGlobalData& global,
                               QVector<SceneObject>& objects,
                               const QList<CS123SceneLightData>& lights,
                               KdTree* tree,
                               AABB extends,
                               int curIndex,
                               int count);

/**
 * @brief computeObjectColor: compute the color at specific position
 * @param objectIndex: object index in the object list
 * @param objects: object list
 * @param global: global scene data
 * @param lights: light data
 * @param tree: pointer to the tree
 * @param extends: bounding box of the scene
 * @param pos: position of intersection
 * @param norm: normal at that position
 * @param eyePos: eye position
 * @param texture: texture color
 * @return: result color
 */
CS123SceneColor computeObjectColor(const int& objectIndex,
                                   QVector<SceneObject>& objects,
                                   const CS123SceneGlobalData& global,
                                   const QList<CS123SceneLightData>& lights,
                                   KdTree* tree,
                                   AABB extends,
                                   const Vector4& pos,
                                   const Vector3& norm,
                                   const Vector4& eyePos,
                                   const  CS123SceneColor& texture);
#endif // TRACE_H
