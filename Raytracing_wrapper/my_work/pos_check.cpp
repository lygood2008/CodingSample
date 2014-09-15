/*!
    @file pos_check.cpp
    @desc: declarations of the functions doing position check (check if the
            position is inside object)
    @author: yanli
    @date: May 2013
 */

#include "pos_check.h"

QVector<SceneObject> checkPos(const QVector<SceneObject>& objects,
                              const Vector4& pos,
                              QMap<int, int >& indexMap)
{

    QVector<SceneObject> result;

    for (int i = 0; i < objects.size(); i++)
    {
        Vector4 posInObjSpace = objects[i].m_invTransform*pos;
        bool in = false;
        switch (objects[i].m_primitive.type)
        {
        case PRIMITIVE_CUBE:
            in  = checkCube(posInObjSpace);
            break;
        case PRIMITIVE_CONE:
            in  = checkCone(posInObjSpace);
            break;
        case PRIMITIVE_CYLINDER:
            in  = checkCylinder(posInObjSpace);
            break;
        case PRIMITIVE_SPHERE:
            in  = checkSphere(posInObjSpace);
            break;
        case PRIMITIVE_TORUS:
            break;
        case PRIMITIVE_MESH:
            break;
        default:
            assert(0);
            break;
        }
        if (in)
        {
            result.push_back(objects[i]);
            indexMap.insert(result.size() - 1, i);
        }
    }

    return result;
}

bool checkCube(const Vector4& posInObjSpace)
{

    Vector3 pos3 = Vector3(posInObjSpace.x / posInObjSpace.w,
                           posInObjSpace.y / posInObjSpace.w,
                           posInObjSpace.z / posInObjSpace.w);

    return pos3.x < 0.5 &&
            pos3.x > -0.5 &&
            pos3.y < 0.5 &&
            pos3.y > -0.5 &&
            pos3.z < 0.5 &&
            pos3.z > -0.5 ? true: false;
}

bool checkCylinder(const Vector4& posInObjSpace )
{

    Vector3 pos3 = Vector3(posInObjSpace.x / posInObjSpace.w,
                           posInObjSpace.y / posInObjSpace.w,
                           posInObjSpace.z / posInObjSpace.w);

    Vector3 pos3H = Vector3(pos3.x, 0, pos3.z);

    return pos3H.length() < 0.5 && pos3.y >-0.5 && pos3.y < 0.5 ? true : false;
}

bool checkCone(const Vector4& posInObjSpace)
{

    Vector3 pos3 = Vector3(posInObjSpace.x / posInObjSpace.w,
                           posInObjSpace.y / posInObjSpace.w,
                           posInObjSpace.z / posInObjSpace.w);


    if (pos3.y > -0.5 && pos3.y < 0.5)
    {
        Vector3 pos3H = Vector3(pos3.x, 0, pos3.z);
        float length = 0.25 - 0.5 * pos3.y;
        return pos3H.length() < length ? true : false;
    }

   return false;
}

bool checkSphere(const Vector4& posInObjSpace)
{

    Vector3 pos3 = Vector3(posInObjSpace.x / posInObjSpace.w,
                           posInObjSpace.y / posInObjSpace.w,
                           posInObjSpace.z / posInObjSpace.w);

    return pos3.length() < 0.5? true : false;
}

