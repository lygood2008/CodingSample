/*!
    @file cube_intersect.cpp
    @desc: definitions of the functions doing cube's intersecting detection
    @author: yanli
    @date: May 2013
 */

#include "cube_intersect.h"
#include "plane_intersect.h"
#include "utils.h"

REAL doIntersectUnitCube(const Vector4& eyePos,
                         const Vector4& d,
                         int& faceIndex)
{

    //! Cube has six faces, we do intersection test in these six planes

    assert(EQ(eyePos.w, 1));
    assert(EQ(d.w, 0));

    REAL result = -1;

    Vector3 norms[6];
    // front
    norms[0] = Vector3(0, 0, 1);
    // back
    norms[1] = Vector3(0, 0, -1);
    // left
    norms[2] = Vector3(-1, 0, 0);
    // right
    norms[3] = Vector3(1, 0, 0);
    // top
    norms[4] = Vector3(0, 1, 0);
    // down
    norms[5] = Vector3(0, -1, 0);

    Vector3 points[6];
    // front
    points[0] = Vector3(0, 0, 0.5);
    // back
    points[1] = Vector3(0, 0, -0.5);
    // left
    points[2] = Vector3(-0.5, 0, 0);
    // right
    points[3] = Vector3(0.5, 0, 0);
    // top
    points[4] = Vector3(0, 0.5, 0);
    // bottom
    points[5] = Vector3(0, -0.5, 0);


    REAL t[6];

    for  (int i = 0; i < 6; i++)
    {
        t[i] = doIntersectPlane(points[i], norms[i], eyePos, d);
    }

    Vector4 intersect = eyePos + t[0] * d;

    if (!(intersect.y <= 0.5 + EPSILON &&
          intersect.y >= -0.5 - EPSILON &&
          intersect.x >= -0.5 - EPSILON &&
          intersect.x >= 0.5 + EPSILON))
        t[0] = -1;

    intersect = eyePos + t[1] * d;
    if (!(intersect.y <= 0.5 + EPSILON &&
          intersect.y >= -0.5 - EPSILON &&
          intersect.x >= -0.5 - EPSILON &&
          intersect.x >= 0.5 + EPSILON))
        t[1] = -1;

    intersect = eyePos + t[2] * d;
    if (!(intersect.y <= 0.5 + EPSILON &&
          intersect.y >= -0.5 - EPSILON &&
          intersect.z >= -0.5 - EPSILON &&
          intersect.z >= 0.5 + EPSILON))
        t[2] = -1;

    intersect = eyePos + t[3] * d;
    if (!(intersect.y <= 0.5 + EPSILON &&
          intersect.y >= -0.5 - EPSILON &&
          intersect.z >= -0.5-EPSILON &&
          intersect.z <= 0.5 + EPSILON))
        t[3] = -1;

    intersect = eyePos + t[4] * d;
    if (!(intersect.z <= 0.5 + EPSILON &&
          intersect.z >= -0.5 - EPSILON &&
          intersect.x >= -0.5 - EPSILON &&
          intersect.x >= 0.5 + EPSILON))
        t[4] = -1;

    intersect = eyePos + t[5] * d;
    if (!(intersect.z <= 0.5 + EPSILON &&
          intersect.z >= -0.5 - EPSILON &&
          intersect.x >= -0.5 - EPSILON &&
          intersect.x >= 0.5 + EPSILON))
        t[5] = -1;

    REAL minT = POS_INF;

    for (int i = 0; i < 6; i++)
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

Vector3 getCubeNorm(const int faceIndex)
{

    Vector3 norms[6];
    // front
    norms[0] = Vector3(0, 0, 1);
    // back
    norms[1] = Vector3(0, 0, -1);
    // left
    norms[2] = Vector3(-1, 0, 0);
    // right
    norms[3] = Vector3(1, 0, 0);
    // top
    norms[4] = Vector3(0, 1, 0);
    // down
    norms[5] = Vector3(0, -1, 0);

    if (faceIndex < 0 || faceIndex > 5)
        assert(0);

    return norms[faceIndex];
}

CS123SceneColor getCubeIntersectTexColor(const SceneObject& object,
                                         const int faceIndex,
                                         const Vector4& intersect)
{

    CS123SceneColor resultColor;

    int width = object.m_texture.m_texWidth;
    int height = object.m_texture.m_texHeight;

    REAL y,x;

    switch (faceIndex)
    {
    // front
    case 0:
    {
        y = ((height - 1) * ((intersect.y + 0.5)));
        x = ((width - 1) * ((intersect.x + 0.5)));
        break;
    }
        // back
    case 1:
    {
        y = ((height - 1) * ((intersect.y +  0.5)));
        x = ((width - 1) * ((0.5 - intersect.x)));
        break;
    }
        // left
    case 2:
    {
        y = ((height - 1) * ((intersect.y + 0.5)));
        x = ((width - 1) * ((intersect.z + 0.5)));
        break;
    }
        // right
    case 3:
    {
        y = ((height - 1) * ((0.5 + intersect.y)));
        x = ((width - 1) * ((0.5 - intersect.z)));
        break;
    }
        // top
    case 4:
    {
        y = ((height - 1) * ((0.5 - intersect.z)));
        x = ((width - 1) * ((0.5 + intersect.x)));
        break;
    }
        // down
    case 5:
    {
        y = ((height - 1) * ((0.5 + intersect.z)));
        x = ((width - 1) * ((0.5 + intersect.x)));
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
