/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/

#include "CS123Common.h"
#include "camera.h"
#include <GL/glu.h>

OrbitCamera::OrbitCamera()
{
    m_up = Vector3(0.f, 1.f, 0.f);
    m_zoom = 2.50f;
    m_theta = 0.f;
    m_phi = 0.f;
    m_fovy = 60.f;
    m_near = 0.1f;
    m_far = 500.0f;
    m_ratio = 1.0f;

    m_panX = 0.f;
    m_panY = 0.f;

    m_oldX = 0;
    m_oldY = 0;
    updateMatrices();
}

OrbitCamera::~OrbitCamera()
{
    //nothing to release
}

void OrbitCamera::mouseDown(const int x, const int y)
{
    m_oldX = x;
    m_oldY = y;
}

void OrbitCamera::mouseMove( int x, int y )
{
    Vector2 delta;
    delta.y = y - m_oldY;
    delta.x = x - m_oldX;
    m_oldX = x;
    m_oldY = y;

    m_theta += delta.x * 0.01f;
    m_phi += delta.y * 0.01f;

    m_theta -= floorf(m_theta / (2*M_PI)) * (2*M_PI);
    m_phi = max((float)(0.01f - M_PI / 2), min((float)(M_PI / 2 - 0.01f), m_phi));
    updateModelviewMatrix();
}

void OrbitCamera::mouseMovePan(const int x, const int y)
{
    m_panY += (y - m_oldY)*0.01;
    m_panX += (x - m_oldX)*0.01;
    m_oldX = x;
    m_oldY = y;

    updateModelviewMatrix();
}

void OrbitCamera::mouseWheel(float delta)
{
    m_zoom *= powf(0.999f, delta);
    updateModelviewMatrix();
}

void OrbitCamera::updateMatrices()
{
    updateProjectionMatrix();
    updateModelviewMatrix();
}

void OrbitCamera::updateProjectionMatrix()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float matrix[16];
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(m_fovy, m_ratio, m_near, m_far);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();
    //because it's column order
    m_projectionMatrix = Matrix4x4(matrix).getTranspose();
}

void OrbitCamera::updateModelviewMatrix()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float matrix[16];
    //double matrixT[16];
    //double matrixR1[16];
    //double matrixR2[16];
    glPushMatrix();
    glLoadIdentity();

    // Move the object forward by m_zoomZ units before we rotate, so it will rotate about a point in front of us
    //glTranslatef(0, 0, m_zoom );
    // Now rotate the object, pivoting it about the new origin in front of us
    // Move the object forward by m_zoomZ units before we rotate, so it will rotate about a point in front of us
    glTranslatef(0, 0, -m_zoom );

    glRotatef(m_phi*180/M_PI, 1, 0, 0);
    glRotatef(m_theta*180/M_PI, 0, 1, 0);

//    glTranslatef( m_panX, m_panY, 0 );
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glPopMatrix();

    // Get the transformed up vector
    // becaues it's column order
    //Matrix4x4 tmp = Matrix4x4(matrix).getTranspose();
    //Vector4 upTrans4= tmp*Vector4( m_up.x, m_up.y, m_up.z, 1 );
    //Vector3 upTrans3 = Vector3( upTrans4.x, upTrans4.y, upTrans4.z ).unit();
    // Get the direction
    Vector3 look(-Vector3::fromAngles(m_theta+M_PI/2.f, m_phi));

    Vector3 w = -look.unit();
    Vector3 v  = m_up - (m_up.dot(w))*w;
    v = v.unit();
    Vector3 u = v.cross(w);
    u = u.unit();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLoadMatrixf( matrix );

    glTranslatef( m_panY*v.x + m_panX*u.x, m_panY*v.y + m_panX*u.y, m_panY*v.z + m_panX*u.z );

    glGetFloatv( GL_MODELVIEW_MATRIX, matrix );
    glPopMatrix();

    m_modelviewMatrix = Matrix4x4(matrix).getTranspose();
}

void OrbitCamera::applyPerspectiveCamera( const int width, const int height)
{
    if( width != 0 && height != 0 )
        setRatio(width/(float)height);

    Vector3 dir(-Vector3::fromAngles(m_theta+M_PI/2.f, m_phi));
    Vector4 eye4 = getEyePos();
    Vector3 eye = Vector3( eye4.x, eye4.y, eye4.z );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(m_fovy, m_ratio, m_near, m_far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye.x, eye.y, eye.z, eye.x + dir.x, eye.y + dir.y, eye.z + dir.z,
              m_up.x, m_up.y, m_up.z);
}

Matrix4x4 OrbitCamera::getInvViewTransMatrix()
{

    Matrix4x4 invModelView = m_modelviewMatrix.getInverse();

    Matrix4x4 invScale = Matrix4x4::identity();

    REAL m_heightAngle = m_fovy;
    REAL heightRadians = m_heightAngle/360*M_PI;
    REAL h = m_far*tan(heightRadians);
    REAL w = h*m_ratio;

    invScale.data[0] = w;
    invScale.data[5] = h;
    invScale.data[10] = m_far;

    Matrix4x4 invViewTransMat = invModelView*invScale;
    return invViewTransMat;
}

Vector4 OrbitCamera::getEyePos()
{
    Vector4 eyePos;
    eyePos = m_modelviewMatrix.getInverse()*Vector4(0,0,0,1);
    return eyePos;
}

Vector3 OrbitCamera::getDir()
{
    return -Vector3::fromAngles(m_theta, m_phi);
}

Vector3 OrbitCamera::getUp()
{
    return m_up;
}

void OrbitCamera::setRatio(float ratio)
{
    m_ratio = ratio;
    updateProjectionMatrix();
}

void OrbitCamera::update( const int width, const int height )
{
    applyPerspectiveCamera( width, height );
}

void OrbitCamera::update()
{
    applyPerspectiveCamera( 0, 0 );
}
