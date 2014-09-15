/*!
    @file plane_intersect.h
    @desc: declarations of the functions doing plane's intersecting detection
    @author: yanli
    @date: May 2013
 */

#ifndef PLANE_INTERSECT_H
#define PLANE_INTERSECT_H


#include "CS123Algebra.h"
#include "vector.h"

/**
 * @brief doIntersectPlane: do intersection detection with plane
 * @param point: one point on the plane
 * @param norm: the normal of the plane
 * @param eyePos: the eye position
 * @param d: the direction
 * @return: the 't' value
 */
REAL doIntersectPlane(const Vector3& point,
                      const Vector3& norm,
                      const Vector4& eyePos,
                      const Vector4& d);

#endif // PLANE_INTERSECT_H
