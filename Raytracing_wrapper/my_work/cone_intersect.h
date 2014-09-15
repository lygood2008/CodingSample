/*!
    @file cone_intersect.h
    @desc: declarations of the functions doing cone's intersecting detection
    @author: yanli
    @date: May 2013
 */

#ifndef CONE_INTERSECT_H
#define CONE_INTERSECT_H

#include "vector.h"
#include "scene.h"

/**
 * @brief doIntersectUnitCone: return the 't' value for the line given by
 *                             eye
 * @param eyePos: eye position
 * @param d: eye direction
 * @param faceIndex: the face index of cone
 * @return: the 't' value
 */
REAL doIntersectUnitCone(const Vector4& eyePos,
                         const Vector4& d,
                         int& faceIndex);
/**
 * @brief getConeNorm: get the normal vector from the intersection point
 * @param intersectPoint: the intersection point
 * @param faceIndex: the face index for cone
 * @return: the normal vector
 */
Vector3 getConeNorm(const Vector4& intersectPoint,
                    const int faceIndex);

/**
 * @brief getConeIntersectTexColor: get the texture color from intersection point
 * @param object: the scene object
 * @param faceIndex: the face index of cone
 * @param intersect: the intersection point
 * @return: the color
 */
CS123SceneColor getConeIntersectTexColor(const SceneObject& object,
                                         const int faceIndex,
                                         const Vector4& intersect);
#endif
