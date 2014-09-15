/*!
    @file cube_intersect.h
    @desc: declarations of the functions doing cube's intersecting detection
    @author: yanli
    @date: May 2013
 */

#ifndef CUBE_INTERSECT_H
#define CUBE_INTERSECT_H

#include "vector.h"
#include "scene.h"

/**
 * @brief doIntersectUnitCube: return the 't' value for the line given by
 *                             eye
 * @param eyePos: eye position
 * @param d: eye direction
 * @param faceIndex: the face index of cube
 * @return: the 't' value
 */
REAL doIntersectUnitCube(const Vector4& eyePos,
                         const Vector4& d,
                         int& faceIndex);
/**
 * @brief getCubeNorm: get the normal vector from the intersection point
 * @param faceIndex: the face index for cube
 * @return: the normal vector
 */
Vector3 getCubeNorm(const int faceIndex);

/**
 * @brief getCubeIntersectTexColor: get the texture color from intersection point
 * @param object: the scene object
 * @param faceIndex: the face index of cube
 * @param intersect: the intersection point
 * @return: the color
 */
CS123SceneColor getCubeIntersectTexColor(const SceneObject& object,
                                         const int faceIndex,
                                         const Vector4& intersect);
#endif
