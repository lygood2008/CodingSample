/*!
    @file resource_loader.h
    @desc: declarations of texture loaders
    @author: yanli
    @date: May 2013
 */

#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QString>
#include <qgl.h>

/**
 * loadTexture: load the texture from a path
 * @param path: the path of the texture
 * @return: the texture handle
 */
GLuint loadTexture(const QString &path);

/**
 * @brief createTexture: create a texture
 * @param texName: the texture handle
 * @param sizeX: size in X dim
 * @param sizeY: size in Y dim
 */
void createTexture(GLuint *texName, const int sizeX, const int sizeY);

/**
 * @brief loadBMPTexture: load BMP texture, this function is not written by me
 * @param texName: the texture handle
 * @param filename: the file name
 * @return: the status for loading, true for success and false for failure
 */
bool loadBMPTexture(GLuint* texName, const char* filename);

#endif // RESOURCELOADER_H
