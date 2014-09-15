/*!
    @file sphere_intersect.h
    @desc: declarations of the functions doing sphere's intersecting detection
    @author: yanli
    @date: May 2013
 */

#ifndef SPHERE_INTERSECT_H
#define SPHERE_INTERSECT_H

#include "vector.h"
#include "scene.h"

/**
 * @brief doIntersectUnitSphere: return the 't' value for the line given by
 *                             eye
 * @param eyePos: eye position
 * @param d: eye direction
 * @return: the 't' value
 */
REAL doIntersectUnitSphere(const Vector4& eyePos,
                           const Vector4& d);
/**
 * @brief getSphereNorm: get the normal vector from the intersection point
 * @param intersectPoint: the position of intersection point
 * @return: the normal vector
 */
Vector3 getSphereNorm(const Vector4& intersectPoint);

/**
 * @brief getSphereIntersectTexColor: get the texture color from the
 *                                   intersection point
 * @param object: the scene object
 * @param intersect: the intersection point
 * @return: the color
 */
CS123SceneColor getSphereIntersectTexColor(const SceneObject& object,
                                           const Vector4& intersectPoint);

#endif
