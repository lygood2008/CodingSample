/*!
    @file kdtreenode.h
    @desc: the declarations of KdtreeNode class
    @author: yanli
    @date: May 2013
 */

#ifndef KDTREENODE_H
#define KDTREENODE_H

#include "kdtreecommon.h"

/**
 * @class: KdTreeNode
 * @brief The KdTreeNode class is the node structure of kdtree
 */
class KdTreeNode
{
public:

    KdTreeNode();
    ~KdTreeNode(){}

    /**
     * @brief add: add an object to the kdtree
     * @param object: the pointer to the scene object
     * @param kdtree: the pointer to the kdtree
     */
    void add(SceneObject* object, KdTree* kdtree);

    /**
     * Setters
     */
    void setAxis(int axis) { m_axis = axis; }
    void setSplitPos(float pos) { m_split = pos; }
    void setLeft(KdTreeNode* left) { m_left = left; }
    void setRight(KdTreeNode* right) { m_right = right; }
    void setLeaf(bool leaf) { m_leaf = leaf; }
    void setObjectList(ObjectNode* node) { m_objList = node; }
    void setAABB(AABB box){ m_box = box; }

    /**
     * Getters
     */
    int getAxis() { return m_axis; }
    float getSplitPos() { return m_split; }
    KdTreeNode* getLeft() { return m_left;  }
    KdTreeNode* getRight() { return m_right; }
    bool isLeaf() { return m_leaf; }
    ObjectNode* getObjectList() { return  m_objList; }
    AABB getAABB(){return m_box; }

private:

    AABB m_box; // Bounding box
    float m_split; // Split position
    bool m_leaf; // Is leaf?
    int m_axis; // Which axis?

    KdTreeNode* m_left; // Left pointer
    KdTreeNode* m_right; // Right pointer
    ObjectNode* m_objList; // Object list
};

#endif // KDTREENODE_H
