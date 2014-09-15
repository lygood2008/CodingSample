/*!
    @file shapeDraw.h
    @desc: the declarations of basic functions drawing primitives
    @author: yanli
    @date: May 2013
 */

#ifndef SHAPEDRAW_H
#define SHAPEDRAW_H

#include <qgl.h>

/**
 * External declarations of tessellation variables
 */

extern int cubeTess;
extern int sphereTess[2];
extern int cylinderTess;
extern int coneTess;

/**
 * @brief buildCubeVBO: build cube's vertex buffer object
 * @param tess: tesselation parameter
 * @param vbo: vbo handle
 * @param vboIndex: index buffer object handle
 */
void buildCubeVBO(int tess, GLuint & vbo, GLuint & vboIndex);

/**
 * @brief buildCylinderVBO: build cylinder's vertex buffer object
 * @param tess: tesselation parameter
 * @param vbo: vbo handle
 * @param vboIndex: index buffer object handle
 */
void buildCylinderVBO(int tess, GLuint& vbo, GLuint& vboIndex);

/**
 * @brief buildConeVBO: build cone's vertex buffer object
 * @param tess: tesselation parameter
 * @param vbo: vbo handle
 * @param vboIndex: index buffer object handle
 */
void buildConeVBO(int tess, GLuint& vbo, GLuint& vboIndex);

/**
 * @brief buildSphereVBO
 * @param tess1: tesselation parameter 1
 * @param tess2: tesselation parameter 2
 * @param vbo: vbo handle
 * @param vboIndex: index buffer object handle
 */
void buildSphereVBO(int tess1, int tess2, GLuint& vbo, GLuint& vboIndex);

/**
 * @brief drawCube: draw a cube
 * @param vbo: vbo handle
 * @param vboElement: index buffer object handle
 * @param texHandle: texture handle
 */
void drawCube(GLuint vbo, GLuint vboElement, GLuint texHandle = 0);

/**
 * @brief drawCylinder: draw a cylinder
 * @param vbo: vbo handle
 * @param vboElement: index buffer object handle
 * @param texHandle: texture handle
 */
void drawCylinder(GLuint vbo, GLuint vboElement, GLuint texHandle = 0);

/**
 * @brief drawCone: draw a cone
 * @param vbo: vbo handle
 * @param vboElement: index buffer object handle
 * @param texHandle: texture handle
 */
void drawCone(GLuint vbo, GLuint vboElement, GLuint texHandle = 0);

/**
 * @brief drawSphere: draw a sphere
 * @param vbo: vbo handle
 * @param vboElement: index buffer object handle
 * @param texHandle: texture handle
 */
void drawSphere(GLuint vbo, GLuint vboElement, GLuint texHandle = 0);

/**
 * @brief drawNormals: draw normals
 * @param vbo: vbo handle
 * @param vertNum: vertext number
 */
void drawNormals(GLuint vbo, int vertNum);

#endif // SHAPEDRAW_H
