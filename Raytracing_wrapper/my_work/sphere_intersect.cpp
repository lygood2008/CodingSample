/*!
    @file sphere_intersect.cpp
    @desc: definitions of the functions doing sphere's intersecting detection
    @author: yanli
    @date: May 2013
  */

#include "sphere_intersect.h"
#include "utils.h"

REAL doIntersectUnitSphere(const Vector4& eyePos,
                           const Vector4& d)
{

    assert(EQ(eyePos.w, 1));
    assert(EQ(d.w, 0));

    // Sphere has only one part that needs to be checked!
    REAL result = -1;
    REAL t = -1;
    REAL A = d.x * d.x + d.y * d.y + d.z * d.z;
    REAL B = 2 * eyePos.x * d.x + 2 * eyePos.y * d.y +
            2 * eyePos.z * d.z;
    REAL C = eyePos.x * eyePos.x + eyePos.y * eyePos.y +
            eyePos.z * eyePos.z - 0.25;

    REAL determinant = B * B - 4 * A * C;

    if (determinant >= 0)
    {
        REAL t1 = (-B + sqrt(determinant)) / (2 * A);
        REAL t2 = (-B - sqrt(determinant)) / (2 * A);
        if (t1 > 0 && t2 <= 0)
            t = t1;
        else if (t1 <= 0 && t2 >0)
            t = t2;
        else if (t1 > 0 && t2 >0)
            t = MIN(t1, t2);
    }

    if (t > 0)
    {
        result = t;
    }

    return result;
}

Vector3 getSphereNorm(const Vector4& intersectPoint)
{

    Vector3 norm = Vector3(intersectPoint.x / intersectPoint.w,
                           intersectPoint.y / intersectPoint.w,
                           intersectPoint.z / intersectPoint.w);

    return norm.unit();
}

CS123SceneColor getSphereIntersectTexColor(const SceneObject& object,
                                            const Vector4& intersectPoint
                                           )
{

    CS123SceneColor resultColor;

    int width  = object.m_texture.m_texWidth;
    int height = object.m_texture.m_texHeight;
    REAL y,x;

    Vector4 Vn  = Vector4(0, 1, 0, 0);
    Vector4 Ve  = Vector4(1, 0, 0, 0);
    Vector4 Vp  = intersectPoint;

    Vp = Vp.unhomgenize();
    Vp = Vp.getNormalized();
    REAL phi = acos(-Vn.dot(Vp));
    REAL v = phi / M_PI;

    REAL theta = (acos(Vp.dot(Ve) / sin(phi))) / (2 * M_PI);

    theta = atan2(Vp.z, Vp.x);

    if (theta < 0)
    {
        theta = theta + 2 * M_PI;
    }
    REAL u = 1- theta / (2 * M_PI);
    x = ((width - 1) * u);
    y = ((height - 1) * v);

    resultColor = bilinearInterpTexel(object.m_texture.m_texPointer,
                                      x,
                                      y,
                                      width,
                                      height);
    resultColor.a = 0;
    return resultColor;
}
