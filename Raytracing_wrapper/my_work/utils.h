/*!
    @file utils.h
    @desc: declarations of utility functions
    @author: yanli
    @date: May 2013
 */

#ifndef UTILS_H
#define UTILS_H

#include "CS123SceneData.h"

/**
 * @brief printMat4x4: print 4x4 matrix
 */
void printMat4x4(char* title, const Matrix4x4& mat);

/**
 * @brief bilinearInterpTexel: do bilateral interpolation
 */
RGBA bilinearInterpTexel(const unsigned * texPixels,
                          float x, float y, const int width,
                          const int height, const int offset = 0);

/**
 * color coversion
 */
RGBA rgbaUintToColor(const unsigned int rgba);
unsigned rgbaColorToUint(RGBA rgba);

/**
 * @brief getReflectionDir: compute the reflection direction
 */
Vector4 getReflectionDir(Vector3 norm, Vector4 incident);

/**
 * @brief getRefracetionDir: compute the refraction direction
 */
Vector4 getRefracetionDir(Vector3 norm, Vector4 incident, REAL n1, REAL n2);

/**
 * mclamp: do clamping number
 */
template <typename T>
inline void mclamp(T& in, T bottom, T top)
{
    assert(top >= bottom);

    if(in > top)
        in = top;
    else if(in < bottom)
        in = bottom;
}

/**
 * @brief isInString: string match
 * @param string: the string
 * @param search: the pattern
 * @return: true if search in string, otherwise false
 */
bool isInString(char* string, const char* search);

#endif // UTILS_H
