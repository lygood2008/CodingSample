/*!
 @file shapeDraw.cpp
 @desc: the definitions of basic functions drawing primitives
 @author: yanli
 @date: May 2013
 */
#include <iostream>
#include <assert.h>
#include <cmath>

#include "shape_draw.h"
#include "vector.h"
#include "global.h"

#include <GL/glext.h>
#include <GL/glut.h>

extern "C"
{
    void glBindBuffer(GLenum target, GLuint buffer);
    void glGenBuffers(GLsizei n, GLuint *buffers);
    void glBufferData(GLenum target, GLsizeiptr size,
                      const GLvoid *data, GLenum usage);
    void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
                         const GLvoid * data);
    void glGetBufferParameteriv(GLenum target, GLenum value, GLint * data);
    void *glMapBuffer(GLenum target, GLenum access);
    void *glUnmapBuffer(GLenum target);
    void glDeleteBuffers(GLsizei n, const GLuint* buffers);
}
/**
 * Tesselation parameter
 */
int cubeTess;
int sphereTess[2];
int cylinderTess;
int coneTess;

void buildCubeVBO(int tess, GLuint &vbo, GLuint &vboIndex)
{

    // The minimum tessellation is 1
    if(tess < 1)
        tess = 1;
    cubeTess = tess;

    const float unit          = 1.f / cubeTess;
    const int sizeVertices    = (cubeTess + 1) * (cubeTess + 1) * 6;
    const int sizeInd         = (cubeTess + 2) * cubeTess * 2 * 6;
    const int sizeVboEachFace = sizeVertices / 6;
    const int sizeIndEachFace = sizeInd / 6;
    Vector3* vertices         = new Vector3[sizeVertices];
    Vector3* normals          = new Vector3[sizeVertices];
    Vector2* texCoords        = new Vector2[sizeVertices];
    GLuint* indices           = new GLuint[sizeInd];

    // Firstly set the normals
    int index = 0;

    // Top faces
    Vector3 base    = Vector3(-0.5, 0.5, -0.5);
    Vector2 baseTex = Vector2(0.f, 1.f);

    for(int i = 0; i < cubeTess + 1; i++)
    {
        for(int j = 0; j < cubeTess + 1; j++)
        {

            vertices[index]  =
                    base + j * Vector3(unit,0, 0) + i * Vector3(0, 0, unit);
            normals[index]   = Vector3(0.f, 1.f, 0.f);
            texCoords[index] =
                    baseTex + j * Vector2(unit, 0) + i * Vector2(0, -unit);
            index++;
        }
    }

    // Bottom faces
    base    = Vector3(0.5, -0.5, 0.5);
    baseTex = Vector2(1.f, 1.f);

    for(int i = 0; i < cubeTess + 1; i++)
    {
        for(int j = 0; j < cubeTess + 1; j++)
        {
            vertices[index]  =
                    base + j * Vector3(-unit, 0, 0) + i * Vector3(0, 0, -unit);
            normals[index]   = Vector3(0.f, -1.f, 0.f);
            texCoords[index] = \
                    baseTex + j * Vector2(-unit, 0) + i * Vector2(0, -unit);
            index++;
        }
    }

    // Left faces

    base    = Vector3(-0.5, 0.5, -0.5);
    baseTex = Vector2(0.f, 1.f);

    for(int i = 0; i < cubeTess + 1; i++)
    {
        for(int j = 0; j < cubeTess + 1; j++)
        {
            vertices[index]  =
                    base + j * Vector3(0, 0, unit) + i * Vector3(0, -unit, 0);
            normals[index]   = Vector3(-1.f, 0.f, 0.f);
            texCoords[index] =
                    baseTex + j * Vector2(unit, 0) + i * Vector2(0, -unit);
            index++;
        }
    }

    // Right faces
    base = Vector3(0.5, -0.5, 0.5);
    baseTex = Vector2(0.f, 0.f);

    for(int i = 0; i < cubeTess + 1; i++)
    {
        for(int j = 0; j < cubeTess + 1; j++)
        {
            vertices[index]  =
                    base + j * Vector3(0, 0, -unit) + i * Vector3(0, unit, 0);
            normals[index]   = Vector3(1.f, 0.f, 0.f);
            texCoords[index] =
                    baseTex + j * Vector2(unit, 0) + i * Vector2(0, unit);
            index++;
        }
    }

    // Front faces
    base = Vector3(-0.5, 0.5, 0.5);
    baseTex = Vector2(0.f, 1.f);

    for(int i = 0; i < cubeTess + 1; i++)
    {
        for(int j = 0; j < cubeTess + 1; j++)
        {
            vertices[index]  =
                    base + j * Vector3(unit, 0, 0) + i * Vector3(0, -unit, 0);
            normals[index]   = Vector3(0.f, 0.f, 1.f);
            texCoords[index] =
                    baseTex + j * Vector2(unit, 0) + i * Vector2(0, -unit);
            index++;
        }
    }

    // Back faces

    base = Vector3(0.5, -0.5, -0.5);
    baseTex = Vector2(0.f, 0.f);

    for(int i = 0; i < cubeTess + 1; i++)
    {
        for(int j = 0; j < cubeTess + 1; j++)
        {
            vertices[index]  =
                    base + j * Vector3(-unit, 0, 0) + i * Vector3(0, unit, 0);
            normals[index]   = Vector3(0.f, 0.f, -1.f);
            texCoords[index] =
                    baseTex + j * Vector2(unit, 0) + i * Vector2(0, unit);
            index++;
        }
    }

    assert(index == sizeVertices);
    // Top index
    index = 0;
    for(int i = 0; i < cubeTess; i++)
    {
        int j;
        for(j =  0; j < cubeTess+1; j++)
        {
            indices[index] = j + i * (cubeTess + 1);
            index++;
            indices[index] = j + (i + 1) * (cubeTess + 1);
            index++;
        }
        indices[index] = j - 1 + (i + 1) * (cubeTess + 1);
        index++;
        indices[index] = 0 + (i + 1) * (cubeTess + 1);
        index++;
    }

    assert(sizeIndEachFace == index);

    for(int i = 1; i < 6; i++)
    {
        for(int j = 0; j < sizeIndEachFace; j++)
        {
            indices[i * sizeIndEachFace + j] = indices[j] + i * sizeVboEachFace;
        }
    }
    // Create the vbo
    GLuint newBuffer;
    glGenBuffers(1, &newBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newBuffer);
    glBufferData(GL_ARRAY_BUFFER, (sizeVertices + sizeVertices) *
                 sizeof(Vector3) + sizeVertices * sizeof(Vector2),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices * sizeof(Vector3),
                    vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector3), normals);
    glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector2), texCoords);

    // Check the size
    int bufSize;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE_ARB, &bufSize);
    if((unsigned)bufSize != sizeVertices * 2 * sizeof(Vector3) + sizeVertices *
            sizeof(Vector2))
    {
        std::cerr << "[buildCubeVBO] data size of vbo buffer is"
                  << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        delete []vertices;
        delete []normals;
        delete []indices;
        delete []texCoords;

        return;
    }

    vbo = newBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create the element buffer
    GLuint eleBuffer;
    glGenBuffers(1, &eleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * sizeInd,
                 indices, GL_STATIC_DRAW);

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);

    if((unsigned)bufSize != sizeInd * sizeof(GLuint))
    {
        std::cerr<< "[buildCubeVBO] data size of element buffer is"
                 << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;
        return;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vboIndex = eleBuffer;


    delete []vertices;
    delete []normals;
    delete []texCoords;
    delete []indices;
}

void buildCylinderVBO(int tess, GLuint& vbo, GLuint& vboIndex)
{

    if(tess < 3)
        tess = 3;

    cylinderTess = tess;
    // Top face
    const int sizeVertices = (cylinderTess + 1) * 4;
    const int sizeInd      = (cylinderTess + 1) * 2 + (cylinderTess + 2) * 2;

    Vector3* vertices      = new Vector3[sizeVertices];
    Vector3* normals       = new Vector3[sizeVertices];
    Vector2* texCoords     = new Vector2[sizeVertices];

    GLuint* indices        = new GLuint[sizeInd];
    const float unitTheta  = 2 * M_PI / cylinderTess;

    int index;

    vertices[0]  = Vector3(0.f, 0.5f, 0.f);
    normals[0]   = Vector3(0.f, 1.f, 0.f);
    texCoords[0] = Vector2(0.5, 0.5);
    // Top faces
    for(index = 1; index < cylinderTess + 1; index++)
    {
        vertices[index].x = 0.5 * cos(unitTheta * index);
        vertices[index].z = 0.5 * sin(unitTheta * index);
        vertices[index].y = 0.5;
        texCoords[index]  = Vector2(0.5 + 0.5 * cos(2 * M_PI - unitTheta * index),
                                    0.5 + 0.5 * sin(2 * M_PI - unitTheta * index));
        normals[index]    = Vector3(0.f, 1.f, 0.f);
    }

    // Bottom faces
    vertices[index]  = Vector3(0.f, -0.5f, 0.f);
    normals[index]   = Vector3(0.f, -1.f, 0.f);
    texCoords[index] = Vector2(0.5,0.5);
    index++;

    for(; index < 2 * (cylinderTess + 1); index++)
    {
        vertices[index].x = 0.5 * cos(unitTheta * index);
        vertices[index].z = 0.5 * sin(unitTheta * index);
        vertices[index].y = -0.5;
        texCoords[index]  = Vector2(0.5 + 0.5 * cos(2 * M_PI - unitTheta * index),
                                    0.5 + 0.5 * sin(2 * M_PI - unitTheta * index));
        normals[index]    = Vector3(0.f, -1.f, 0.f);
    }

    // Side faces
    int base = index;
    for(; index < 3 * (cylinderTess + 1); index++)
    {
        vertices[index].x = 0.5 * cos(unitTheta * (index-base));
        vertices[index].z = 0.5 * sin(unitTheta * (index-base));
        vertices[index].y = 0.5;
        texCoords[index]  = Vector2((2 * M_PI-unitTheta * (index-base)) /
                                    (2 * M_PI), 1);
        normals[index]    = Vector3(cos(unitTheta * (index-base)), 0.f,
                                    sin(unitTheta * (index-base)));
    }

    base = index;
    for(; index < sizeVertices; index++)
    {
        vertices[index].x = 0.5 * cos(unitTheta * (index-base));
        vertices[index].z = 0.5 * sin(unitTheta * (index-base));
        vertices[index].y = -0.5;
        texCoords[index]  = Vector2((2 * M_PI-unitTheta * (index-base)) /
                                    (2 * M_PI), 0);
        normals[index]    = Vector3(cos(unitTheta * (index-base)), 0.f,
                                    sin(unitTheta * (index-base)));
    }

    /**
     Use triangle fan to draw the top face and bottom face
     **/
    index = 0;
    indices[index++] = 0;
    for(int i = 0; i < cylinderTess + 1; i++)
    {
        indices[index] = (i + 1) >= (cylinderTess + 1) ? 1 : (i + 1);
        index++;
    }

    indices[index++] = cylinderTess + 1;
    for(int i = cylinderTess + 1; i < 2 * (cylinderTess + 1); i++)
    {
        indices[index] =
                (i + 1) >= 2 * (cylinderTess + 1) ? cylinderTess + 2 : (i + 1);
        index++;
    }

    base = 2 * (cylinderTess + 1);
    for(int i = 0; i < (cylinderTess + 1); i++)
    {

        indices[index] = base + i;
        index++;
        indices[index] = (cylinderTess + 1) + base + i ;
        index++;
    }

    assert(index == sizeInd);

    GLuint newBuffer;
    glGenBuffers(1, &newBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newBuffer);
    glBufferData(GL_ARRAY_BUFFER, (sizeVertices + sizeVertices) *
                 sizeof(Vector3) + sizeVertices * sizeof(Vector2),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices * sizeof(Vector3), vertices);

    glBufferSubData(GL_ARRAY_BUFFER, sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector3), normals);
    glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector2), texCoords);
    int bufSize;

    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE_ARB, &bufSize);

    if((unsigned)bufSize != sizeVertices * 2 * sizeof(Vector3) + sizeVertices *
            sizeof(Vector2))
    {
        std::cerr << "[buildCylinderVBO] data size of vbo buffer is"
                  << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;

        return;
    }

    vbo = newBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create the element buffer
    GLuint eleBuffer;
    glGenBuffers(1, &eleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * sizeInd, indices,
                 GL_STATIC_DRAW);

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);

    if((unsigned)bufSize != sizeInd * sizeof(GLuint))
    {
        std::cerr<< "[buildCylinderVBO] data size of element buffer is"
                 << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;
        return;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vboIndex = eleBuffer;


    delete []vertices;
    delete []normals;
    delete []texCoords;
    delete []indices;
}

void buildConeVBO(int tess, GLuint& vbo, GLuint& vboIndex)
{

    if(tess < 3)
        tess = 3;

    coneTess = tess;
    // Top face
    const int sizeVertices = (coneTess + 1) * 3;
    const int sizeInd      = (coneTess + 1) * 4;

    Vector3* vertices      = new Vector3[sizeVertices];
    Vector3* normals       = new Vector3[sizeVertices];
    Vector2* texCoords     = new Vector2[sizeVertices];

    GLuint* indices        = new GLuint[sizeInd];
    const float unitTheta  = 2 * M_PI / coneTess;

    int index = 0;

    float normalSlope = 1.0 / 2.0;
    float normalV     = normalSlope / sqrt(1 + normalSlope * normalSlope);
    float normalH     = 1.0 / sqrt(1 + normalSlope * normalSlope);

    for(int i = 0; i < coneTess + 1; i++, index++)
    {

        vertices[index]  = Vector3(0.f, 0.5f, 0.f);
        normals[index]   = Vector3(normalH * cos(unitTheta * i),
                                   normalV,
                                   normalH * sin(unitTheta * i));
        texCoords[index] = Vector2((2 * M_PI- unitTheta * (i)) / (2 * M_PI), 1);
    }

    for(int i = 0; i < coneTess + 1; i++, index++)
    {
        vertices[index]  = Vector3(0.5 * cos(unitTheta * i),
                                   -0.5f,
                                   0.5 * sin(unitTheta * i));
        normals[index]   = Vector3(normalH * cos(unitTheta * i),
                                   normalV,
                                   normalH * sin(unitTheta * i));
        texCoords[index] = Vector2((2 * M_PI - unitTheta * (i)) / (2 * M_PI),
                                   0);
    }

    vertices[index]  = Vector3(0.f, -0.5f, 0.f);
    normals[index]   = Vector3(0.f, -1.f, 0.f);
    texCoords[index] = Vector2(0.5, 0.5);
    index++;
    for(int i = 0; i < coneTess; i++, index++)
    {

        vertices[index].x = 0.5 * cos(unitTheta * i);
        vertices[index].z = 0.5 * sin(unitTheta * i);
        vertices[index].y = -0.5;

        normals[index]    = Vector3(0.f, -1.f, 0.f);
        texCoords[index]  = Vector2(0.5 + 0.5 * cos(unitTheta * i),
                                    0.5 + 0.5 * sin(unitTheta * i));
    }

    assert(index == sizeVertices);

    index = 0;
    for(int i = 0; i < coneTess + 1; i++)
    {
        indices[index] = i;
        index++;
        indices[index] = coneTess + 1 + i;
        index++;
    }

    const int base = 2 * (coneTess + 1);
    for(int i = 0; i < coneTess + 1; i++)
    {
        indices[index] = base;
        index++;
        indices[index] = (i >= coneTess) ? base + 1 : base + i + 1;
        index++;
    }

    assert(index == sizeInd);
    GLuint newBuffer;
    glGenBuffers(1, &newBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newBuffer);
    glBufferData(GL_ARRAY_BUFFER, (sizeVertices + sizeVertices) *
                 sizeof(Vector3) + sizeVertices * sizeof(Vector2),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices * sizeof(Vector3),
                    vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector3), normals);
    glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector2), texCoords);
    int bufSize;

    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE_ARB, &bufSize);

    if((unsigned)bufSize != sizeVertices * 2 * sizeof(Vector3) +
            sizeVertices * sizeof(Vector2))
    {
        std::cerr<< "[buildConeeVBO] data size of vbo buffer is"
                 << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;

        return;
    }

    vbo = newBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create the element buffer
    GLuint eleBuffer;
    glGenBuffers(1, &eleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * sizeInd, indices,
                 GL_STATIC_DRAW);

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);

    if((unsigned)bufSize != sizeInd * sizeof(GLuint))
    {
        std::cerr<< "[buildConeVBO] data size of element buffer is"
                 << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;
        return;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vboIndex = eleBuffer;


    delete []vertices;
    delete []normals;
    delete []texCoords;
    delete []indices;
}

void buildSphereVBO(int tess1, int tess2, GLuint &vbo, GLuint &vboIndex)
{

    if(tess1 < 3)
        tess1 = 3;
    if(tess2 < 3)
        tess2 = 3;

    sphereTess[0] = tess1;
    sphereTess[1] = tess2;

    const int sizeVertices = (sphereTess[1] + 1) * (sphereTess[0] + 1);
    const int sizeInd      = (sphereTess[1]) * (sphereTess[0]) * 3 * 2;
    Vector3* vertices      = new Vector3[sizeVertices];
    Vector3* normals       = new Vector3[sizeVertices];
    Vector2* texCoords     = new Vector2[sizeVertices];
    GLuint* indices        = new GLuint[sizeInd];

    const float unitTheta  = 2 * M_PI / sphereTess[1];
    const float unitPhi    = M_PI / sphereTess[0];
    int index = 0;

    for(int i = 0; i < sphereTess[0] + 1; i++)
    {
        for(int j = 0; j < sphereTess[1] + 1; j++)
        {
            vertices[index]  = Vector3(0.5 * sin(unitPhi * i) * cos(unitTheta * j),
                                       0.5 * cos(unitPhi * i),
                                       0.5 * sin(unitPhi * i) * sin(unitTheta * j));
            normals[index]   = Vector3(sin(unitPhi * i) * cos(unitTheta * j),
                                       cos(unitPhi * i),
                                       sin(unitPhi * i) * sin(unitTheta * j));
            texCoords[index] = Vector2((2 * M_PI - unitTheta * j) / (2 * M_PI),
                                       (M_PI - unitPhi * i) / (M_PI));
            index++;
        }
    }
    assert(index == sizeVertices);

    index = 0;

    for(int i = 0; i < sphereTess[0]; i++)
    {
        for(int j = 0; j < sphereTess[1]; j++)
        {
            indices[index] = i * (sphereTess[1] + 1) + j;
            index++;
            indices[index] = (i + 1) * (sphereTess[1] + 1) + j;
            index++;
            indices[index] = (i + 1) * (sphereTess[1] + 1) + (j + 1);
            index++;

            indices[index] = i * (sphereTess[1] + 1)  +  j;
            index++;
            indices[index] = (i + 1) * (sphereTess[1] + 1) + (j + 1);
            index++;
            indices[index] = (i) * (sphereTess[1] + 1) + (j + 1);
            index++;
        }
    }

    assert(index == sizeInd);

    GLuint newBuffer;
    glGenBuffers(1, &newBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newBuffer);

    glBufferData(GL_ARRAY_BUFFER, (sizeVertices + sizeVertices) *
                 sizeof(Vector3) + sizeVertices * sizeof(Vector2),
                 NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeVertices * sizeof(Vector3),
                    vertices);

    glBufferSubData(GL_ARRAY_BUFFER, sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector3), normals);

    glBufferSubData(GL_ARRAY_BUFFER, 2 * sizeVertices * sizeof(Vector3),
                    sizeVertices * sizeof(Vector2), texCoords);

    int bufSize;

    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE_ARB, &bufSize);

    if((unsigned)bufSize != (sizeVertices + sizeVertices) * sizeof(Vector3) +
            sizeVertices * sizeof(Vector2))
    {
        std::cerr<< "[buildSphereVBO] data size of vbo buffer is"
                 << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;

        return;
    }

    vbo = newBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create the element buffer
    GLuint eleBuffer;
    glGenBuffers(1, &eleBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * sizeInd, indices,
                 GL_STATIC_DRAW);

    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufSize);

    if((unsigned)bufSize != sizeInd * sizeof(GLuint))
    {
        std::cerr<< "[buildSphereVBO] data size of element buffer is"
                 << "mismatch with input array\n";
        vbo = 0;
        vboIndex = 0;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete []vertices;
        delete []normals;
        delete []texCoords;
        delete []indices;
        return;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    vboIndex = eleBuffer;


    delete []vertices;
    delete []normals;
    delete []texCoords;
    delete []indices;
}

void drawCube(GLuint vbo, GLuint vboElement, GLuint texHandle)
{

    const int verticesArrayLength    = (cubeTess + 1) * (cubeTess + 1) * 6;
    const int indicesArrayLengthEach = (cubeTess + 2) * (cubeTess) * 2;

    glMatrixMode(GL_MODELVIEW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glNormalPointer(GL_FLOAT, 0, OFFSET(verticesArrayLength * sizeof(Vector3)));
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    if(texHandle && settings.showTexture)
    {
        glTexCoordPointer(2, GL_FLOAT, 0,
                          OFFSET(2 * verticesArrayLength * sizeof(Vector3)));
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texHandle);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);



    glFrontFace(GL_CCW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)0);
    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)NULL + indicesArrayLengthEach);
    glFrontFace(GL_CCW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)NULL + indicesArrayLengthEach + 2);
    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)NULL + indicesArrayLengthEach + 3);
    glFrontFace(GL_CCW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)NULL + indicesArrayLengthEach + 4);
    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)NULL + indicesArrayLengthEach + 5);

    if(texHandle)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glFrontFace(GL_CCW);
    PRINT_GL_ERROR();
}

void drawCylinder(GLuint vbo, GLuint vboElement, GLuint texHandle)
{

    const int verticesArrayLength      = (cylinderTess + 1) + 4;
    const int indicesArrayLengthTopBot = cylinderTess + 2;
    const int indicesArrayLengthEach   = (cylinderTess + 1) + 2;

    glMatrixMode(GL_MODELVIEW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glNormalPointer(GL_FLOAT, 0, OFFSET(verticesArrayLength + sizeof(Vector3)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    // If texture handle is zero, then we skip texture binding
    if(texHandle && settings.showTexture)
    {
        glTexCoordPointer(2, GL_FLOAT, 0,
                          OFFSET(2 + verticesArrayLength + sizeof(Vector3)));
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texHandle);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLE_FAN, indicesArrayLengthTopBot, GL_UNSIGNED_INT,
                   (GLuint*)0);
    glFrontFace(GL_CCW);
    glDrawElements(GL_TRIANGLE_FAN, indicesArrayLengthTopBot, GL_UNSIGNED_INT,
                   (GLuint*)0 + indicesArrayLengthTopBot);
    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)0 + indicesArrayLengthTopBot + 2);

    if(texHandle && settings.showTexture)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glFrontFace(GL_CCW);
    PRINT_GL_ERROR();
}

void drawCone(GLuint vbo, GLuint vboElement, GLuint texHandle)
{

    const int verticesArrayLength    = (coneTess + 1) + 3;
    const int indicesArrayLengthEach = (coneTess + 1) + 2;

    glMatrixMode(GL_MODELVIEW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glNormalPointer(GL_FLOAT, 0, OFFSET(verticesArrayLength + sizeof(Vector3)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    // If texture handle is zero, then we skip texture binding
    if(texHandle && settings.showTexture)
    {
        glTexCoordPointer(2, GL_FLOAT, 0,
                          OFFSET(2 + verticesArrayLength + sizeof(Vector3)));
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texHandle);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)0);
    glFrontFace(GL_CCW);
    glDrawElements(GL_TRIANGLE_STRIP, indicesArrayLengthEach, GL_UNSIGNED_INT,
                   (GLuint*)0 + indicesArrayLengthEach);

    if(texHandle && settings.showTexture)
    {
        // Reset texture
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glFrontFace(GL_CCW);
    PRINT_GL_ERROR();
}

void drawSphere(GLuint vbo, GLuint vboElement, GLuint texHandle)
{

    const int verticesArrayLength = (sphereTess[1] + 1) + (sphereTess[0] + 1);
    glMatrixMode(GL_MODELVIEW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glNormalPointer(GL_FLOAT, 0, OFFSET(verticesArrayLength + sizeof(Vector3)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    // If texture handle is zero, then we skip texture binding
    if(texHandle && settings.showTexture)
    {
        glTexCoordPointer(2, GL_FLOAT, 0,
                          OFFSET(2 + verticesArrayLength + sizeof(Vector3)));
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texHandle);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glFrontFace(GL_CW);
    glDrawElements(GL_TRIANGLES, (sphereTess[1]) + (sphereTess[0]) + 3 + 2,
                   GL_UNSIGNED_INT, (GLuint*)0);

    if(texHandle && settings.showTexture)
    {
        // Reset texture
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glFrontFace(GL_CCW);

    PRINT_GL_ERROR();
}

void drawNormals(GLuint vbo, int vertNum)
{

    if(vbo == 0)
    {
        std::cerr<<"[drawNormals] invalid vbo"<<std::endl;
    }
    if(vertNum < 0)
    {
        std::cerr<<"[drawNormals] invalid vertNum"<<std::endl;
    }

    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    Vector3* buffer = (Vector3*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    PRINT_GL_ERROR();

    Vector3* verts = buffer;
    Vector3* norms = buffer + vertNum;

    const float mag = 0.3;
    for(int i = 0; i < vertNum; i++)
    {

        glBegin(GL_LINES);
        glNormal3f(norms[i].x, norms[i].y, norms[i].z);
        glVertex3f(verts[i].x, verts[i].y, verts[i].z);
        glVertex3f(verts[i].x + mag + norms[i].x,
                   verts[i].y + mag + norms[i].y,
                   verts[i].z + mag + norms[i].z);

        glEnd();
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    PRINT_GL_ERROR();
}
