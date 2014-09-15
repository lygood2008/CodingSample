/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/*!
   @file   CamtransCamera.cpp
   @author Ben Herila (ben@herila.net)
   @author Evan Wallace (evan.exe@gmail.com)
   @date   Fall 2010

   @brief  This is the perspective camera class you will need to fill in for the
   Camtrans assignment.  See the assignment handout for more details.

   For extra credit, you can also create an Orthographic camera. Create another
   class if you want to do that.

*/

#include "camtrans_camera.h"
#include "global.h"
#include <qgl.h>

CamtransCamera::CamtransCamera()
{
    // @TODO: [CAMTRANS] Fill this in...
    m_eyePosition.x = 2;
    m_eyePosition.y = 0;
    m_eyePosition.z = 0;
    m_eyePosition.w = 1;

    m_aspectRatio = 1;
    m_heightAngle = 60;
    m_up.x = 0;
    m_up.y = 1;
    m_up.z = 0;
    m_up.w = 0;

    m_near = 1;
    m_far = 5;

    m_lookAt.x = -1;
    m_lookAt.y = 0;
    m_lookAt.z = 0;
    m_lookAt.w = 0;

    m_useQuaternion = USEQUATERNION;

    orientLook( m_eyePosition, m_lookAt, m_up );
}

void CamtransCamera::setAspectRatio(float a)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_aspectRatio = a;

    updateProjectionMatrix();
}

Matrix4x4 CamtransCamera::getProjectionMatrix() const
{
    // @TODO: [CAMTRANS] Fill this in...
   /* Matrix4x4 scale = Matrix4x4::identity();

    REAL heightRadians = m_heightAngle/360*M_PI;
    REAL h = m_far*tan(heightRadians);
    REAL w = h*m_aspectRatio;

    scale.data[0] = 1.0/w;
    scale.data[5] = 1.0/h;
    scale.data[10] = 1.0/m_far;


    REAL c = (-1)*(m_near/m_far);
    Matrix4x4 unhinging = Matrix4x4::identity();
    unhinging.data[10] = -1.0/(c+1);
    unhinging.data[11] = ((1.0)*c)/(c+1);
    unhinging.data[14] = -1;
    unhinging.data[15] = 0;

    Matrix4x4 composite = unhinging*scale;
    return composite;*/
    return m_projection;
}

Matrix4x4 CamtransCamera::getModelviewMatrix() const
{
    // @TODO: [CAMTRANS] Fill this in...
    /*Matrix4x4 translate = Matrix4x4::identity();
    translate = getInvTransMat(m_eyePosition);

    Matrix4x4 rotate = Matrix4x4::identity();
    rotate.data[0] = m_u.x;
    rotate.data[1] = m_u.y;
    rotate.data[2] = m_u.z;
    rotate.data[4] = m_v.x;
    rotate.data[5] = m_v.y;
    rotate.data[6] = m_v.z;
    rotate.data[8] = m_n.x;
    rotate.data[9] = m_n.y;
    rotate.data[10] = m_n.z;

    Matrix4x4 composite = rotate*translate;
    return composite;*/
    return m_modelView;
}

Matrix4x4 CamtransCamera::getInvViewTransMatrix() const
{
    Matrix4x4 invModelView = m_modelView.getInverse();

    Matrix4x4 invScale = Matrix4x4::identity();

    REAL heightRadians = m_heightAngle/360*M_PI;
    REAL h = m_far*tan(heightRadians);
    REAL w = h*m_aspectRatio;

    invScale.data[0] = w;
    invScale.data[5] = h;
    invScale.data[10] = m_far;

    Matrix4x4 invViewTransMat = invModelView*invScale;
    return invViewTransMat;

}
Vector4 CamtransCamera::getPosition() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_eyePosition;
}

Vector4 CamtransCamera::getLook() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_lookAt;
}

Vector4 CamtransCamera::getUp() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_up;
}

float CamtransCamera::getAspectRatio() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_aspectRatio;
}

float CamtransCamera::getHeightAngle() const
{
    // @TODO: [CAMTRANS] Fill this in...
    return m_heightAngle;
}

void CamtransCamera::orientLook(const Vector4 &eye, const Vector4 &look, const Vector4 &up)
{
    // @TODO: [CAMTRANS] Fill this in...

    m_n = look;
    m_n = m_n.getNormalized();
    m_n *= (-1);
    m_n.unhomgenize();

    REAL uProjection = m_n.dot( up );
    m_v.x = up.x - uProjection*m_n.x;
    m_v.y = up.y - uProjection*m_n.y;
    m_v.z = up.z - uProjection*m_n.z;
   m_v =  m_v.getNormalized();
    m_v.unhomgenize();

    m_u = m_v;
    m_u = m_v.cross(m_n);
    m_u.unhomgenize();

    m_eyePosition = eye;
    m_lookAt = look;
    m_up = up;

    REAL lN = m_lookAt.getMagnitude();
    m_lookAt /= lN;
    m_lookAt.unhomgenize();

    m_up = m_v;
    m_up.unhomgenize();

    updateModelViewMatrix();
    updateProjectionMatrix();
}

void CamtransCamera::setHeightAngle(float h)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_heightAngle = h;

    updateProjectionMatrix();
}

void CamtransCamera::translate(const Vector4 &v)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_eyePosition = getTransMat( v )*m_eyePosition;

    updateModelViewMatrix();
}

void CamtransCamera::rotateU(float degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    REAL radians = degrees/180*M_PI;
    Matrix4x4 rot = Matrix4x4::identity();
    if( !m_useQuaternion )
        rot = getRotMat( m_eyePosition,m_u,-radians );
    else
        rot = getRotMatFromQuaternion( m_eyePosition,m_u,-radians );

    m_v = rot*m_v;
    m_n = rot*m_n;
    m_lookAt = rot*m_lookAt;
    m_up = rot*m_up;

    updateModelViewMatrix();
}

void CamtransCamera::rotateV(float degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    REAL radians = degrees/180*M_PI;
    Matrix4x4 rot = Matrix4x4::identity();

    if( !m_useQuaternion )
        rot = getRotMat( m_eyePosition,m_v,radians );
    else
        rot = getRotMatFromQuaternion( m_eyePosition,m_v,radians );

    m_u = rot*m_u;
    m_n = rot*m_n;
    m_lookAt = rot*m_lookAt;
    m_up = rot*m_up;

    updateModelViewMatrix();
}

void CamtransCamera::rotateN(float degrees)
{
    // @TODO: [CAMTRANS] Fill this in...
    REAL radians = degrees/180*M_PI;

    Matrix4x4 rot = Matrix4x4::identity();
    if( !m_useQuaternion )
        rot = getRotMat( m_eyePosition,m_n,radians );
    else
        rot = getRotMatFromQuaternion( m_eyePosition,m_n,radians );


    m_u = rot*m_u;
    m_v = rot*m_v;
    m_lookAt = rot*m_lookAt;
    m_up = rot*m_up;

    updateModelViewMatrix();
}

void CamtransCamera::setClip(float nearPlane, float farPlane)
{
    // @TODO: [CAMTRANS] Fill this in...
    m_near = nearPlane;
    m_far = farPlane;

    updateProjectionMatrix();
}

void CamtransCamera::updateProjectionMatrix()
{
    Matrix4x4 scale = Matrix4x4::identity();

    REAL heightRadians = m_heightAngle/360*M_PI;
    REAL h = m_far*tan(heightRadians);
    REAL w = h*m_aspectRatio;

    scale.data[0] = 1.0/w;
    scale.data[5] = 1.0/h;
    scale.data[10] = 1.0/m_far;


    REAL c = (-1)*(m_near/m_far);
    Matrix4x4 unhinging = Matrix4x4::identity();
    unhinging.data[10] = -1.0/(c+1);
    unhinging.data[11] = ((1.0)*c)/(c+1);
    unhinging.data[14] = -1;
    unhinging.data[15] = 0;

    m_projection = unhinging*scale;
}

void CamtransCamera::updateModelViewMatrix()
{
    Matrix4x4 translate = Matrix4x4::identity();
    translate = getInvTransMat(m_eyePosition);

    Matrix4x4 rotate = Matrix4x4::identity();
    rotate.data[0] = m_u.x;
    rotate.data[1] = m_u.y;
    rotate.data[2] = m_u.z;
    rotate.data[4] = m_v.x;
    rotate.data[5] = m_v.y;
    rotate.data[6] = m_v.z;
    rotate.data[8] = m_n.x;
    rotate.data[9] = m_n.y;
    rotate.data[10] = m_n.z;

    m_modelView = rotate*translate;
}
