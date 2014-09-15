/*!
    @file plane_intersect.cpp
    @desc: declarations of the functions doing plane's intersecting detection
    @author: yanli
    @date: May 2013
 */

#include "plane_intersect.h"

REAL doIntersectPlane(const Vector3& point,
                      const Vector3& norm,
                      const Vector4& eyePos,
                      const Vector4& d)
{

    REAL t = -1;
    REAL denominator = norm.x * d.x + norm.y * d.y + norm.z * d.z;
    if(fabs(denominator) > EPSILON)
    {
        t = ((norm.x * point.x + norm.y * point.y + norm.z * point.z) -
             (norm.x * eyePos.x + norm.y * eyePos.y + norm.z * eyePos.z)) /
                (denominator);
    }

    return t;
}
