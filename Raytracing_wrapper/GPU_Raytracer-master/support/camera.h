/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/*!
    @file camera.h
    @date: Fall 2012
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <QMouseEvent>
#include "CS123Algebra.h"
#include "vector.h"

class OrbitCamera
{

public:
    OrbitCamera();
    ~OrbitCamera();
    void mouseDown(const int x, const int y);
    void mouseMove(const int x, const int y);
    void mouseMovePan( const int x, const int y );
    void mouseWheel(const float delta);
    void updateMatrices();
    void updateProjectionMatrix();
    void updateModelviewMatrix();
    void applyPerspectiveCamera(const int width, const int height);
    Matrix4x4 getInvViewTransMatrix();
    Vector4 getEyePos();
    Vector3 getDir();
    Vector3 getUp();
    void setRatio( float ratio );

    inline float getRatio() { return m_ratio; }
    inline float getNear() { return m_near; }
    inline float getFar() { return m_far; }
    inline float getFOV() { return m_fovy; }

    inline Matrix4x4 getProjectionMatrix() { return m_projectionMatrix; }
    inline Matrix4x4 getModelviewMatrix() { return m_modelviewMatrix; }

    void update( const int width, const int height );
    void update();

private:

    Vector3 m_up;
    float m_theta, m_phi;
    float m_fovy;
    float m_zoom;

    float m_ratio;
    float m_near;
    float m_far;
    int m_oldX, m_oldY;

    float m_panX;
    float m_panY;

    Matrix4x4 m_projectionMatrix;
    Matrix4x4 m_modelviewMatrix;
};

#endif // CAMERA_H
