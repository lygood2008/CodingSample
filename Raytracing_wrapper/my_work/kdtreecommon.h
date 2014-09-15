/*!
    @file kdtreelist.h
    @desc: declarations of helper structures for kdtree
    @author: yanli
    @date: May 2013
 */

#ifndef KDTREECOMMON_H
#define KDTREECOMMON_H

#include "scene.h"

/**
 * @struct: SplitNode
 * @brief The SplitNode struct is used for splitting kdtree node when building
 *        kdtree
 */
struct SplitNode
{
    float splitPos; // Split position
    int leftCount; // Objects count in the left side
    int rightCount; // Objects count in the right side
    SplitNode* next; // Next splitPos
};

class SceneObject;
/**
 * @class: ObjectNode
 * @brief The ObjectNode class is used for holding scene objects
 *        It's only useful when creating kdtree
 */
class ObjectNode
{
public:

    ObjectNode() : m_object(NULL), m_next(NULL) {}
    ~ObjectNode() {}

    /**
     * Setters
     */
    void setObject(SceneObject* obj) { m_object = obj; }
    void setNext(ObjectNode* next) { m_next = next; }

    /**
     * Getters
     */
    SceneObject* getObject() { return m_object; }
    ObjectNode* getNext() { return m_next; }

private:

    SceneObject* m_object; // The scene objects in this node
    ObjectNode* m_next; // the pointer to the next object node
};

#endif // KDTREECOMMON_H
