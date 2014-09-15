/*!
    @file aabb.h
    @desc: declarations of the interfaces of AABB (bounding box) class
    @author: yanli
    @date: May 2013
 */

#ifndef AABB_H
#define AABB_H

#include "vector.h"
#include "CS123Algebra.h"

/**
 * @class: AABB
 * @brief The AABB class is responsible for storing the AABB bounding box
 *        information
 */
class AABB
{
public:

    AABB() : m_pos(Vector3(0, 0, 0)), m_size(Vector3(0, 0, 0)) {}
    AABB(Vector3 pos, Vector3 size) : m_pos(pos), m_size(size) {}

    /**
     * @brief intersectAABB: check if two bounding box intersects
     * @param b2: the target bounding box
     * @return: the result of dection
     */
    bool intersectAABB(AABB& b2)
    {
        Vector3 v1 = b2.getPos(),
                v2 = b2.getPos() + b2.getSize();
        Vector3 v3 = m_pos,
                v4 = m_pos + m_size;

        return ((v4.x >= v1.x) && (v3.x <= v2.x) && // x-axis overlap
                (v4.y >= v1.y) && (v3.y <= v2.y) && // y-axis overlap
                (v4.z >= v1.z) && (v3.z <= v2.z));   // z-axis overlap
    }

    /**
     * @brief contains: if the position is inside bouding box
     * @param pos: the 3D position
     * @return: the boolean value for detection
     */
    bool contains(Vector3 pos)
    {
        Vector3 v1 = m_pos,
                v2 = m_pos + m_size;

        return ((pos.x >= v1.x) && (pos.x <= v2.x) &&
                (pos.y >= v1.y) && (pos.y <= v2.y) &&
                (pos.z >= v1.z) && (pos.z <= v2.z));
    }

    /**
     * getters
     */
    inline Vector3& getPos() { return m_pos; }
    inline Vector3& getSize() { return m_size; }
    inline float w() { return m_size.x; }
    inline float h() { return m_size.y; }
    inline float d() { return m_size.z; }
    inline float x() { return m_pos.x; }
    inline float y() { return m_pos.y; }
    inline float z() { return m_pos.z; }

private:

    Vector3 m_pos, m_size; // postion of the AABB box and size of it
};


/**
 * @brief computeCubeAABB: compute the bounding box for cube
 * @param transform: the transformation matrix
 * @return: the result bounding box
 */
AABB computeCubeAABB(const Matrix4x4 transform);

/**
 * @brief computeSphereAABB: compute the bounding box for sphere
 * @param transform: the transformation matrix
 * @return: the result bounding box
 */
AABB computeSphereAABB(const Matrix4x4 transform);

/**
 * @brief computeConeAABB: compute the bounding box for cone
 * @param transform: the transformation matrix
 * @return: the result bounding box
 */
AABB computeConeAABB(const Matrix4x4 transform) ;

/**
 * @brief computeCylinderAABB: compute the bounding box for cylinder
 * @param transform: the transformation matrix
 * @return: the result bounding box
 */
AABB computeCylinderAABB(const Matrix4x4 transform);

/**
 * @brief drawAABB: draw the bounding box
 * @param aabb: the boudning box
 * @param color: the color in rgb
 */
void drawAABB(AABB& aabb, float color[3]);

#endif
