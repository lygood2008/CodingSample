/*!
 @file intersect.h
 @desc: declarations of the functions doing intersect detection.
        Basically the function is the wrapper for doing intersect test
        on all objects.
 @author: yanli
 @date: May 2013
 */


#ifndef INTERSECT_H
#define INTERSECT_H


#include "aabb.h"
#include "CS123Algebra.h"
#include "scene.h"

class KdTree;

/**
 * @brief intersect: the wrapper for doing intersecting detection on
 *                   all possible objects
 * @param eyePos: eye position
 * @param objects: the list of objects
 * @param d: eye direction
 * @param objectIndex: the object index, should be returned
 * @param faceIndex: the face index, should be returned
 * @param tree: the pointer to the kdtree
 * @param extends: the bounding box of the whole scene
 * @param refract: enabling refraction or not
 * @return: the 't' value
 */
REAL intersect(const Vector4& eyePos,
               const QVector<SceneObject>& objects,
               const Vector4& d,
               int& objectIndex,
               int& faceIndex,
               KdTree* tree,
               AABB extends,
               bool refract = false);

/**
 * @brief doIntersect: do intersecting detection on specific object
 * @param eyePos: eye position
 * @param d: eye direction
 * @param faceIndex: the face index, should be returned
 * @return: the 't' value
 */
REAL doIntersect(const SceneObject &object,
                 const Vector4& eyePos,
                 const Vector4& d,
                 int& faceIndex);

#endif // INTERSECT_H
