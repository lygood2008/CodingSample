/*!
    @file kdbox_intersect.h
    @desc: declarations of the functions doing kdtree box's intersecting
           detection
    @author: yanli
    @date: May 2013
 */

#ifndef KDBOX_INTERSECT_H
#define KDBOX_INTERSECT_H

#include "CS123Algebra.h"
#include "aabb.h"

/**
 * @brief doIntersectRayKdBox: do the intersecting detection with KD box
 * @param eyePos: the eye position
 * @param d: the direction
 * @param near: the 't' value intersecting near plane
 * @param far: the 't' value intersecting far plane
 * @param box: the AABB box
 */
void doIntersectRayKdBox(const Vector4& eyePos,
                         const Vector4& d,
                         REAL& near,
                         REAL& far,
                         AABB box);

#endif // KDBOX_INTERSECT_H
