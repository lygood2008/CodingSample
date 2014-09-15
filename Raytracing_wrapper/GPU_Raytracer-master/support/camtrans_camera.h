/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/*!
   @file   CamtransCamera.h
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010

   @brief  Contains definitions for an abstract virtual camera class that students
           will implement in the Camtrans assignment.
*/

#ifndef CAMTRANSCAMERA_H
#define CAMTRANSCAMERA_H

#include <CS123Algebra.h>



/**

 @class CamtransCamera

 @brief The perspective camera class students will implement in the Camtrans assignment.

 Part of the CS123 support code.

 @author  Evan Wallace (edwallac)
 @author  Ben Herila (bherila)
 @date    9/1/2010

*/

#define USEQUATERNION true
class Camera
{
public:
    // Will be called when the window size changes
    virtual void setAspectRatio(float) = 0;

    // C++ Note: The following are pure virtual functions - they must be overridden in subclasses.

    //! Return the projection matrix for the current camera settings.
    virtual Matrix4x4 getProjectionMatrix() const = 0;

    //! Return the modelview matrix for the current camera settings.
    virtual Matrix4x4 getModelviewMatrix() const = 0;

    //! Return the inverse viewing transformationg matrix.
    virtual Matrix4x4 getInvViewTransMatrix() const = 0;

    virtual Vector4 getPosition() const = 0;

    //
    // The following functions will be called for mouse events on the 3D canvas tab.
    //
    // C++ Note: Why are the following methods not pure virtual? If they are not overridden in
    // subclasses, then the empty implementation here ({}) will take over.
    //
};

class CamtransCamera : public Camera
{
public:
    //! Initialize your camera.
    CamtransCamera();

    //! Sets the aspect ratio of this camera. Automatically called by the GUI when the window is resized.
    virtual void setAspectRatio(float aspectRatio);

    //! Returns the projection matrix given the current camera settings.
    virtual Matrix4x4 getProjectionMatrix() const;

    //! Returns the modelview matrix given the current camera settings.
    virtual Matrix4x4 getModelviewMatrix() const;

    //! Return the inverse viewing transformationg matrix.
    virtual Matrix4x4 getInvViewTransMatrix() const;

    //! Returns the current position of the camera.
    virtual Vector4 getPosition() const;

    //! Returns the current 'look' vector for this camera.
    Vector4 getLook() const;

    //! Returns the current 'up' vector for this camera.
    Vector4 getUp() const;

    //! Returns the currently set aspect ratio.
    float getAspectRatio() const;

    //! Returns the currently set height angle.
    float getHeightAngle() const;

    //! Orients this camera's look vector with a current eye position, given look and up vectors.
    void orientLook(const Vector4 &eye, const Vector4 &look, const Vector4 &up);

    //! Sets the height angle of this camera.
    void setHeightAngle(float h);

    //! Translates the camera along a given vector.
    void translate(const Vector4 &v);

    //! Rotates the camera about the U axis by a specified number of degrees.
    void rotateU(float degrees);

    //! Rotates the camera about the V axis by a specified number of degrees.
    void rotateV(float degrees);

    //! Rotates the camera about the N axis by a specified number of degrees.
    void rotateN(float degrees);

    //! Sets the near and far clip planes for this camera.
    void setClip(float nearPlane, float farPlane);

    //! update the Projection matrix
    void updateProjectionMatrix();

    //! compute the ModelView matrix
    void updateModelViewMatrix();

    inline void setProjection( REAL aspectRatio, REAL near, REAL far, REAL fov )
    {
        m_aspectRatio = aspectRatio;
        m_near = near;
        m_far = far;
        m_heightAngle = fov;
    }

private:

    Vector4 m_eyePosition;
    Vector4 m_lookAt;
    Vector4 m_up;

    Vector4 m_u;
    Vector4 m_v;
    Vector4 m_n;

    REAL m_aspectRatio;
    REAL m_near;
    REAL m_far;
    REAL m_heightAngle;

    Matrix4x4 m_projection;
    Matrix4x4 m_modelView;

    bool m_useQuaternion;
};

#endif // CAMTRANSCAMERA_H
