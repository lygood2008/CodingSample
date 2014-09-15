/*!
 @file intersect.cpp
 @desc: definitions of the functions doing intersect detection.
 @author: yanli
 @date: May 2013
 */

#include "intersect.h"

#include "cone_intersect.h"
#include "cube_intersect.h"
#include "sphere_intersect.h"
#include "cylinder_intersect.h"
#include "plane_intersect.h"
#include "kdbox_intersect.h"

#include "global.h"
#include "kdtree.h"

QVector<KdTreeNodeHost> kdnode_test;
QVector<ObjectNodeHost> objnode_test;

REAL intersect(const Vector4& eyePos,
               const QVector<SceneObject>& objects,
               const Vector4& d,
               int& objectIndex,
               int& faceIndex,
               KdTree* tree,
               AABB extends,
               bool refract)
{

    REAL minT         = POS_INF;
    REAL resultT      = -1;
    int tempFaceIndex = -1;

    if (settings.useKdTree && tree && !refract)
    {
        assert(EQ(eyePos.w, 1) && EQ(d.w, 0));

        REAL near = -1, far = -1;
        doIntersectRayKdBox(eyePos, d, near, far, extends);
        if (near <= 0 && far <= 0)
            return -1;

        QVector<KdTreeNode*> nodeStack;
        KdTreeNode* current = tree->getRoot();

        while (current)
        {
            while (!current->isLeaf())
            {
                AABB curBox = current->getAABB();
                REAL near, far;
                doIntersectRayKdBox(eyePos, d, near, far, curBox);

                int axis       = current->getAxis();
                float splitPos = current->getSplitPos();

                if (near == far)  // inside the box
                    near = 0;
                Vector4 nearPos = eyePos + d * near;
                Vector4 farPos  = eyePos + d * far;
                if (nearPos.data[axis] <= splitPos)
                {
                    if (farPos.data[axis] <= splitPos)
                    {
                        current = current->getLeft();
                        continue;
                    }
                    KdTreeNode* right = current->getRight();
                    current = current->getLeft();

                    // Preserve the right
                    nodeStack.push_back(right);
                }
                else
                {
                    if (farPos.data[axis] > splitPos)
                    {
                        current = current->getRight();
                        continue;
                    }
                    KdTreeNode* left = current->getLeft();
                    current = current->getRight();
                    nodeStack.push_back(left);
                }
            }

            // Then we found an near leaf, find if there is intersect
            ObjectNode* objList = current->getObjectList();
            minT = POS_INF;

            while (objList)
            {
                SceneObject* curObj  = objList->getObject();
                Matrix4x4 invCompMat = (*curObj).m_invTransform;

                Vector4 eyePosObjSpace = invCompMat * eyePos;
                Vector4 dObjSpace      = invCompMat * d;

                REAL t = doIntersect(*curObj,
                                     eyePosObjSpace,
                                     dObjSpace,
                                     tempFaceIndex);
                if (t > 0 && t < minT)
                {
                    minT = t;
                    objectIndex = curObj->m_arrayID;
                    faceIndex = tempFaceIndex;
                }
                objList = objList->getNext();
            }
            Vector4 dst = eyePos + minT * d;
            // Here we need to enlarge the bounding box a little bit
            AABB curBox = AABB(current->getAABB().getPos() -
                               Vector3(EPSILON,EPSILON, EPSILON),
                               current->getAABB().getSize() +
                               2 * Vector3(EPSILON, EPSILON, EPSILON));

            if (minT != POS_INF &&
                    curBox.contains(Vector3(dst.x, dst.y, dst.z)))
            {
                resultT = minT;
                break;
            }
            // Else we need to get one node from the stack
            if (nodeStack.empty())
            {
                // No more nodes, meaning there is no intersect
                break;
            }
            else
            {
                current = nodeStack.at(nodeStack.size() - 1);
                nodeStack.pop_back();
            }
        }
    }
    else
    {
        for (int i = 0; i < objects.size(); i++)
        {
            const SceneObject& curObj = objects[i];
            Matrix4x4 invCompMat = curObj.m_invTransform;

            Vector4 eyePosObjSpace = invCompMat * eyePos;
            Vector4 dObjSpace      = invCompMat * d;

            REAL t = doIntersect(curObj, eyePosObjSpace,
                                 dObjSpace, tempFaceIndex);
            if (t > 0 && t < minT)
            {
                minT = t;
                objectIndex = i;
                faceIndex = tempFaceIndex;
            }
        }
        if (minT != POS_INF)
        {
            resultT = minT;
        }
    }
    return resultT;
}

REAL doIntersect(const SceneObject &object,
                 const Vector4& eyePos,
                 const Vector4& d,
                 int& faceIndex)
{

    REAL t = -1;
    switch (object.m_primitive.type)
    {
    case PRIMITIVE_CUBE:
    {
        t = doIntersectUnitCube(eyePos, d, faceIndex);
        break;
    }
    case PRIMITIVE_CONE:
    {
        t = doIntersectUnitCone(eyePos, d, faceIndex);
        break;
    }
    case PRIMITIVE_CYLINDER:
    {
        t = doIntersectUnitCylinder(eyePos, d, faceIndex);
        break;
    }
    case PRIMITIVE_SPHERE:
    {
        t = doIntersectUnitSphere(eyePos, d);
        break;
    }
    case PRIMITIVE_TORUS:
    {
        // not support torus
        //assert(0);
        break;
    }
    case PRIMITIVE_MESH:
    {
        // not support mesh
        //assert(0);
        break;
    }
    default:
    {
        assert(0);
        break;
    }
    }
    return t;
}
