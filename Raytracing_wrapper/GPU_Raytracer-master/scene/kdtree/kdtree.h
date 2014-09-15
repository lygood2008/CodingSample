/*!
    @file kdtree.h
    @desc: declarations of KdTree class
    @author: yanli
    @date: May 2013
 */

#ifndef KDTREE_H
#define KDTREE_H

#include "kdtreecommon.h"
#include "kdtreenode.h"

#define MAX_TREE_DEPTH 32 // Maximum depth of the tree
#define KDTREE_ARRAY_SIZE 100000 // We use an array to store the kd tree

/**
 * @class: KdTree
 * @brief The KdTree class is the wrapper class of all kdtree operations
 */
class KdTree
{
public:

    KdTree();
    ~KdTree();

    /**
     * @brief init: initialize memory
     */
    void init();
    /**
     * @brief build: build a kdtree from the scene
     * @param scene: the pointe to the scene
     */
    void build(Scene* scene);

    /**
     * @brief insertSplitPos: insert a split plane at split pos
     * @param splitPos: the split position (1D)
     */
    void insertSplitPos(float splitPos);

    /**
     * @brief subdivide: subdivide the kdtree node based on bounding box,
     *                   depth, object numbers
     * @param node: the pointer to the node
     * @param aabb: the bounding box
     * @param depth: the current depth
     * @param objCount: the object numbers
     */
    void subdivide(KdTreeNode* node, AABB aabb, int depth, int objCount);

    /**
     * @brief newObjectNode: new an object node
     * @return: the pointe to the new object node
     */
    ObjectNode* newObjectNode();

    /**
     * @brief newKdTreeNodePair: new a pair of kdTreeNode
     * @return: the pointer to the pair
     */
    KdTreeNode* newKdTreeNodePair();

    /**
     * @brief newKdTreeNode
     * @return
     */
    KdTreeNode* newKdTreeNode();

    /**
     * setters
     */
    void setRoot(KdTreeNode* root) { m_root = root; }
    /**
     * Getters
     */
    KdTreeNode* getRoot() { return m_root; }
    KdTreeNode* getKdTreeNodeArray() { return m_kdMem; }
    ObjectNode* getObjectNodeArray() { return m_objMem; }

private:

    /**
     * @brief allocateMem: allocate fixed amount of memory
     */
    void allocateMem();

    /**
     * @brief freeObjectNode: free the memory for an object node
     * @param node: the pointer to the object node
     */
    void freeObjectNode(ObjectNode* node);

    /**
     * @brief freeMem: free all
     */
    void freeMem();

    /**
     * @brief calculateAABBRange: calculate AABB's range
     * @param aabb: the parent bounding box
     * @param left: the left boundary
     * @param right: the right bounday
     * @param axis: current axis
     */
    void calculateAABBRange(AABB aabb, float& left,
                             float& right, const int axis);

    /**
     * @brief calculateSA: calculate the surface area
     * @param aabb: the bounding box
     * @return: the surface area
     */
    inline float calculateSA(AABB aabb){ return 2 * (aabb.w() * aabb.d() +
                                                       aabb.w() * aabb.h() +
                                                       aabb.d() * aabb.h()); }
private:

    bool m_allocated; // Allocated?

    KdTreeNode* m_root; // Pointer to the root node
    SplitNode* m_splitList; // Pointer to the split list
    SplitNode*m_splitPool; // Pointer to the empty split node

    KdTreeNode* m_kdPtr; // Just a pointer to the current end of memory for
                         // kdtree
    ObjectNode* m_objPtr; // Just a pointer to the current end of memory for obj

    KdTreeNode* m_kdMem; // Pointer to the memory of all the nodes in byte
    ObjectNode* m_objMem; // Pointer to the memory of
                          // all the object nodes in byte
};

/**
 * @brief dumpKdTreeInfo: helper function for debugging. Dump the information
 *                        of kdtree
 * @param tree: the pointer to the tree
 */
void dumpKdTreeInfo(KdTree* tree);

/**
 * @brief dumpKdTreeInfoLeaf: dump the leaf information
 * @param node: the kdtree node
 * @param ofile: output stream
 * @param depth: the specified depth
 */
void dumpKdTreeInfoLeaf(KdTreeNode* node, std::ofstream& ofile, int depth);

#endif
