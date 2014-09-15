/*!
    @file kdbox_intersect.cpp
    @desc: definition of the functions doing kdtree box's intersecting
           detection
    @author: yanli
    @date: May 2013
 */

#include "kdbox_intersect.h"
#include "plane_intersect.h"

static bool checkRange(const Vector4 intersect,
                       const Vector4 range,
                       const int axis)
{

    bool result = false;
    switch (axis)
    {
    case 0: // X axis range
        if(intersect.y >= range.x && intersect.y <= range.y &&
                intersect.z >= range.z && intersect.z <= range.w)
          result = true;
        break;
    case 1: // Y axis range
        if(intersect.x >= range.x && intersect.x <= range.y &&
                intersect.z >= range.z && intersect.z <= range.w)
          result = true;
        break;
    case 2: // Z axis range
        if(intersect.x >= range.x && intersect.x <= range.y &&
                intersect.y >= range.z && intersect.y <= range.w)
          result = true;
        break;
    default:
        assert(0);
        break;
    }
    return result;
}

void doIntersectRayKdBox(const Vector4& eyePos,
                         const Vector4& d,
                         REAL& near,
                         REAL& far,
                         AABB box)
{

    assert(EQ(eyePos.w, 1) && EQ(d.w, 0));
    near = -1;
    far  = -1;
    Vector3 start = box.getPos();
    Vector3 end   = box.getPos() + box.getSize();
    Vector3 norm[3] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};


    Vector4 range[3] = {
                        Vector4(start.y, end.y, start.z, end.z),
                        Vector4(start.x, end.x, start.z, end.z),
                        Vector4(start.x, end.x, start.y, end.y)
                       };
    REAL t[6];
    t[0] = doIntersectPlane(start, norm[0], eyePos, d);
    t[1] = doIntersectPlane(end, norm[0], eyePos, d);
    t[2] = doIntersectPlane(start, norm[1], eyePos, d);
    t[3] = doIntersectPlane(end, norm[1], eyePos, d);
    t[4] = doIntersectPlane(start, norm[2], eyePos, d);
    t[5] = doIntersectPlane(end, norm[2], eyePos, d);

    Vector4 intersectPoint[6];
    REAL min = POS_INF;
    REAL max = -POS_INF;

    for (int i = 0; i < 6; i++)
    {
        intersectPoint[i] = eyePos + t[i] * d;
        if (t[i] > 0 && checkRange(intersectPoint[i], range[i / 2], i/2) &&
                min > t[i])
        {
            min = t[i];
            near = t[i];
        }
        if (t[i] > 0 && checkRange(intersectPoint[i], range[i / 2], i/2) &&
                max < t[i])
        {
            max = t[i];
            far = t[i];
        }
    }
}
