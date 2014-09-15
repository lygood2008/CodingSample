/*!
    @file pos_check.h
    @desc: declarations of the functions doing position check (check if the
            position is inside object)
    @author: yanli
    @date: May 2013
 */

#ifndef POS_CHECK_H
#define POS_CHECK_H

#include <QVector>
#include "scene.h"
#include <QMap>

/**
 * @brief checkPos: do check pos on the list of objects
 * @param objects: all of the objects
 * @param pos: the position
 * @param indexMap: index map, we need to insert the result into it
 * @return
 */
QVector<SceneObject> checkPos(const QVector<SceneObject>& objects,
                              const Vector4& pos,
                              QMap<int, int>& indexMap );
/**
 * @brief checkCube: check if the position is inside the cube
 * @param posInObjSpace: the position
 * @return: inside or not
 */
bool checkCube(const Vector4& posInObjSpace);

/**
 * @brief checkCylinder: check if the position is inside the cylinder
 * @param posInObjSpace: the position
 * @return: inside or not
 */
bool checkCylinder(const Vector4& posInObjSpace );

/**
 * @brief checkCone: check if the position is inside the cone
 * @param posInObjSpace: the position
 * @return: inside or not
 */
bool checkCone(const Vector4& posInObjSpace);

/**
 * @brief checkSphere: check if the position is inside the sphere
 * @param posInObjSpace: the position
 * @return: inside or not
 */
bool checkSphere(const Vector4& posInObjSpace);

#endif // POS_CHECK_H
