/*!
    @file cylinder_intersect.h
    @desc: declarations of the functions doing cylinder's intersecting detection
    @author: yanli
    @date: May 2013
 */

#ifndef CYLINDER_INTERSECT_H
#define CYLINDER_INTERSECT_H

#include "vector.h"
#include "scene.h"

/**
 * @brief doIntersectUnitCylinder: return the 't' value for the line given by
 *                             eye
 * @param eyePos: eye position
 * @param d: eye direction
 * @param faceIndex: the face index of cylinder
 * @return: the 't' value
 */
REAL doIntersectUnitCylinder(const Vector4& eyePos,
                             const Vector4& d,
                             int& faceIndex);
/**
 * @brief getCylinder: get the normal vector from the intersection point
 * @param intersect point: the intersection point
 * @param faceIndex: the face index for cylinder
 * @return: the normal vector
 */
Vector3 getCylinderNorm(const Vector4& intersectPoint,
                        const int faceIndex);

/**
 * @brief getCylinderIntersectTexColor: get the texture color
 *                                      from intersection point
 * @param object: the scene object
 * @param faceIndex: the face index of cylinder
 * @param intersect: the intersection point
 * @return: the color
 */
CS123SceneColor getCylinderIntersectTexColor(const SceneObject& object,
                                             const int index,
                                             const Vector4& intersect);
#endif
