 /*!
    @file cylinder_intersect.cpp
    @desc: definitions of the functions doing cone's intersecting detection
    @author: yanli
    @date: May 2013
 */
#include "cylinder_intersect.h"
#include "plane_intersect.h"
#include "utils.h"

REAL doIntersectUnitCylinder(const Vector4& eyePos,
                             const Vector4& d,
                             int& faceIndex)
{

    // Cylinder has three parts: top, bottom and side.
    REAL result = -1;
    assert(EQ(eyePos.w, 1));
    assert(EQ(d.w, 0));

    REAL t[3];
    Vector3 norms[3];

    // firstly check if the ray intersects the bottom;
    t[0] = -1;
    norms[0] = Vector3(0, -1, 0);
    t[0] = doIntersectPlane(Vector3(0, -0.5, 0), norms[0], eyePos, d);
    Vector4 intersect = eyePos + t[0] * d;

    if (t[0] != -1 && !(SQ(intersect.x) + SQ(intersect.z) <= 0.25))
        t[0] = -1;

    // secondly check if the ray intersects the top
    t[1] = -1;
    norms[1] = Vector3(0,1,0);
    t[1] = doIntersectPlane(Vector3(0,0.5,0), norms[1], eyePos, d);
    intersect = eyePos + t[1] * d;

    if (t[1] != -1 && !(SQ(intersect.x) + SQ(intersect.z) <= 0.25))
        t[1] = -1;

    // at last check if the ray intersects the side
    t[2] = -1;
    REAL A = d.x * d.x + d.z * d.z;
    REAL B = 2 * eyePos.x * d.x + 2 * eyePos.z * d.z;
    REAL C = eyePos.x * eyePos.x + eyePos.z * eyePos.z - 0.25;
    REAL determinant = B * B - 4 * A * C;

    if(determinant >= 0)
    {
        REAL t1 = (-B + sqrt(determinant)) / (2 * A);
        REAL t2 = (-B - sqrt(determinant)) / (2 * A);

        if(!(eyePos.y + t1 * d.y <= 0.5 + EPSILON &&
             eyePos.y + t1 * d.y >= -0.5 - EPSILON))
        {
            t1 = -1;
        }

        if(!(eyePos.y + t2 * d.y <= 0.5 + EPSILON &&
             eyePos.y + t2 * d.y >= -0.5 - EPSILON))
        {
            t2 = -1;
        }
        if (t1 > 0 && t2 <= 0)
            t[2] = t1;
        else if(t1 <= 0 && t2 >0)
            t[2] = t2;
        else if (t1 > 0 && t2 > 0)
            t[2] = MIN(t1, t2);
    }

    REAL minT = POS_INF;
    for(int i = 0; i < 3; i++)
    {
        if(t[i] > 0 && t[i] < minT)
        {
            minT = t[i];
            faceIndex = i;
        }
    }

    if(minT != POS_INF)
        result = minT;

    return result;
}

Vector3 getCylinderNorm(const Vector4& intersectPoint, const int faceIndex)
{

    Vector3 norms[3];
    norms[0] = Vector3(0, -1, 0);
    norms[1] = Vector3(0, 1, 0);
    norms[2] = Vector3(intersectPoint.x, 0, intersectPoint.z).unit();

    if(faceIndex < 0 || faceIndex > 2)
        assert(0);
    return norms[faceIndex];
}

CS123SceneColor getCylinderIntersectTexColor(const SceneObject& object,
                                             const int index,
                                             const Vector4& intersect)
{

    CS123SceneColor resultColor;

    int width  = object.m_texture.m_texWidth;
    int height = object.m_texture.m_texHeight;
    REAL y,x;
    REAL u,v;
    switch (index)
    {
    // bottom
    case 0:
    {
        u = (0.5 + intersect.x);
        v = 1 - (0.5 - intersect.z);
        x = ((width-1) * u);
        y = ((height-1) * v);

        break;
    }
    // top
    case 1:
    {
        u = (0.5 + intersect.x);
        v = (0.5 - intersect.z);
        x = ((width-1) * u);
        y = ((height-1) * v);
        break;
    }
    case 2:
    {
        Vector4 Ve = Vector4(1, 0, 0, 0);
        Vector4 Vp = Vector4(intersect.x, 0, intersect.z, 0);
        Vp = Vp.getNormalized();
        REAL theta = acos(Vp.dot(Ve));
        if(Vp.z > 0)
            theta = theta + 2 * (M_PI - theta);
        u = theta / (2 * M_PI);
        v = 0.5 + intersect.y;
        x = ((width-1) * u);
        y = ((height-1) * v);
        break;
    }
    default:
        assert(0);
        break;
    }

    resultColor = bilinearInterpTexel(object.m_texture.m_texPointer,
                                      x,
                                      y,
                                      width,
                                      height);
    resultColor.a = 0;
    return resultColor;
}
