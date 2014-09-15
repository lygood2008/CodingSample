/*!
    @file cone_intersect.cpp
    @desc: definitions of the functions doing cone's intersecting detection
    @author: yanli
    @date: May 2013
 */

#include "cone_intersect.h"
#include "plane_intersect.h"
#include "utils.h"

REAL doIntersectUnitCone(const Vector4& eyePos,
                         const Vector4& d,
                         int& faceIndex)
{

    assert(EQ(eyePos.w, 1.f));
    assert(EQ(d.w, 0.f));

    REAL result = -1;
    Vector3 norms[2];
    REAL t[2];

    // firstly check if the ray intersects the bottom;
    t[0] = -1;
    norms[0] = Vector3(0,-1,0);

    t[0] = doIntersectPlane(Vector3(0, -0.5, 0), norms[0], eyePos, d);
    Vector4 intersect = eyePos + t[0] * d;
    if (!(SQ(intersect.x) + SQ(intersect.z) <= 0.25))
        t[0] = -1;

    // then check the intersect point on the side
    t[1] = -1;

    REAL A = d.x * d.x + d.z * d.z -0.25 * d.y * d.y;
    REAL B =
            2 * eyePos.x * d.x +
            2 * eyePos.z * d.z - 0.5 * eyePos.y * d.y + 0.25 * d.y;
    REAL C =
            eyePos.x * eyePos.x + eyePos.z * eyePos.z -
            0.25 * eyePos.y * eyePos.y + 0.25 * eyePos.y - 0.0625;

    REAL determinant = B * B - 4 * A * C;

    if (determinant >= 0)
    {
        REAL t1 = (-B + sqrt(determinant))  / (2 * A);
        REAL t2 = (-B - sqrt(determinant))  / (2 * A);

        if (!(eyePos.y + t1 * d.y <= 0.5+EPSILON &&
              eyePos.y + t1 * d.y >= -0.5-EPSILON))
        {
            t1 = -1;
        }

        if (!(eyePos.y + t2 * d.y <= 0.5+EPSILON &&
              eyePos.y + t2 * d.y >= -0.5-EPSILON))
        {
            t2 = -1;
        }

        if (t1 > 0 && t2 <= 0)
            t[1] = t1;
        else if (t1 <= 0 && t2 > 0)
            t[1] = t2;
        else if (t1 > 0 && t2 > 0)
            t[1] = MIN(t1, t2);
    }
    REAL minT = POS_INF;
    for (int i = 0; i < 2; i++)
    {
        if (t[i] > 0 && t[i] < minT)
        {
            minT = t[i];
            faceIndex = i;
        }
    }
    if (minT != POS_INF)
        result = minT;

    return result;
}

Vector3 getConeNorm(const Vector4& intersect, const int faceIndex)
{

     Vector3 norms[2];
     norms[0] = Vector3(0, -1, 0);
     norms[1] = Vector3(2 * (intersect.x),
                        0.25 - 0.5 * (intersect.y),
                        2 * (intersect.z));

     norms[1] = norms[1].unit();

     if (faceIndex < 0 || faceIndex > 1)
         assert(0);

     return norms[faceIndex];
}

CS123SceneColor getConeIntersectTexColor(const SceneObject& object,
                                         const int faceIndex,
                                         const Vector4& intersect)
{

    CS123SceneColor resultColor;

    int width  = object.m_texture.m_texWidth;
    int height = object.m_texture.m_texHeight;

    REAL x,y;
    REAL u,v;
    switch (faceIndex)
    {
    // bottom
    case 0:
    {
        u = (0.5 + intersect.x);
        v = 1 - (0.5 - intersect.z);
        break;
    }
    case 1:
    {

        v = 0.5 + intersect.y;

        Vector4 Vp = intersect;
        Vp = Vp.unhomgenize();
        Vp = Vp.getNormalized();

        REAL theta = atan2(Vp.z, Vp.x);
        if (Vp.z < 0)
        {
            theta = theta + 2 * M_PI;
        }
        u = 1 - theta  / (2 * M_PI);
        break;
    }
    default:
        assert(0);
        break;
    }

    x = (width-1) * u;
    y = (height-1) * v;

    resultColor = bilinearInterpTexel(object.m_texture.m_texPointer,
                                      x,
                                      y,
                                      width,
                                      height);
    resultColor.a = 0;
    return resultColor;
}
