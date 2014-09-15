/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/*!
   @file   CS123Matrix.cpp
   @author Travis Fischer (fisch0920@gmail.com)
   @date   Fall 2008
   
   @brief
      Provides basic functionality for a templated, arbitrarily-sized matrix.
      You will need to fill this file in for the Camtrans assignment.

**/

#include "CS123Algebra.h"
#include <iostream>

Vector4 getQuaternionFromAngle( const Vector4 &v, const REAL radians )
{
    REAL halfRadians = radians/2;
    Vector4 result;
    result.w = cos(halfRadians);
    REAL s = sin(halfRadians);
    result.x = v.x*s;
    result.y = v.y*s;
    result.z = v.z*s;
    return result;
}

Matrix4x4 getRotMatFromQuaternion( const Vector4 &q )
{
    Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = 1-2*q.y*q.y-2*q.z*q.z;
    m.data[1] = 2*q.x*q.y+2*q.w*q.z;
    m.data[2] = 2*q.x*q.z-2*q.w*q.y;
    m.data[4] = 2*q.x*q.y-2*q.w*q.z;
    m.data[5] = 1-2*q.x*q.x-2*q.z*q.z;
    m.data[6] = 2*q.y*q.z+2*q.w*q.x;
    m.data[8] = 2*q.x*q.z+2*q.w*q.y;
    m.data[9] = 2*q.y*q.z-2*q.w*q.x;
    m.data[10] = 1- 2*q.x*q.x-2*q.y*q.y;
    return m;
}

Matrix4x4 getRotXMatFromQuaternion( const REAL radians)
{
    Vector4 x;
    x.x = 1;
    x.y = 0;
    x.z = 0;
    x.w = 0;
    Vector4 q = getQuaternionFromAngle( x, -radians );
    Matrix4x4 m =  getRotMatFromQuaternion( q );
    return m;
}

Matrix4x4 getRotYMatFromQuaternion( const REAL radians)
{
    Vector4 x;
    x.x = 0;
    x.y = 1;
    x.z = 0;
    x.w = 0;
    Vector4 q = getQuaternionFromAngle( x, -radians );
    Matrix4x4 m =  getRotMatFromQuaternion( q );
    return m;
}

Matrix4x4 getRotZMatFromQuaternion( const REAL radians)
{
    Vector4 x;
    x.x = 0;
    x.y = 0;
    x.z = 1;
    x.w = 0;
    Vector4 q = getQuaternionFromAngle( x, -radians );
    Matrix4x4 m =  getRotMatFromQuaternion( q );
    return m;
}

Matrix4x4 getRotMatFromQuaternion(const Vector4 &p, const Vector4 &v, const REAL a)
{

    Vector4 pTemp = p;
    pTemp.x /= pTemp.w;
    pTemp.y /= pTemp.w;
    pTemp.z /= pTemp.w;
    pTemp.w = 1;

    Vector4 nv = v.getNormalized();
    Vector4 q1 = getQuaternionFromAngle( nv, -a );
    Matrix4x4 m1 =  getRotMatFromQuaternion( q1 );

    Matrix4x4 m = Matrix4x4::identity();

    m = getTransMat(pTemp)*m1*getInvTransMat(pTemp);

    return m;
}


Matrix4x4 getInvRotXMatFromQuaternion( const REAL radians)
{
    return getRotXMatFromQuaternion( -radians );
}

Matrix4x4 getInvRotYMatFromQuaternion( const REAL radians)
{
    return getRotYMatFromQuaternion( -radians );
}

Matrix4x4 getInvRotZMatFromQuaternion( const REAL radians)
{
    return getRotZMatFromQuaternion( -radians );
}

Matrix4x4 getInvRotMatFromQuaternion(const Vector4 &p, const Vector4 &v, const REAL a)
{
    return getRotMatFromQuaternion( p, v, -a);
}

//@name Routines which generate specific-purpose transformation matrices
//@{---------------------------------------------------------------------
// @returns the scale matrix described by the vector
Matrix4x4 getScaleMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = v.x;
    m.data[5] = v.y;
    m.data[10] = v.z;
    return m;
}

// @returns the translation matrix described by the vector
Matrix4x4 getTransMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[3] = v.x;
    m.data[7] = v.y;
    m.data[11] = v.z;
    return m;
}

// @returns the rotation matrix about the x axis by the specified angle
Matrix4x4 getRotXMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[5] = cos(radians);
    m.data[6] = -sin(radians);
    m.data[9] = sin(radians);
    m.data[10] = cos(radians);
    return m;
}

// @returns the rotation matrix about the y axis by the specified angle
Matrix4x4 getRotYMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
   Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = cos(radians);
    m.data[2] = sin(radians);
    m.data[8] = -sin(radians);
    m.data[10] = cos(radians);

    return m;
}

// @returns the rotation matrix about the z axis by the specified angle
Matrix4x4 getRotZMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = cos(radians);
    m.data[1] = -sin(radians);
    m.data[4] = sin(radians);
    m.data[5] = cos(radians);
    return m;
}

// @returns the rotation matrix around the vector and point by the specified angle
Matrix4x4 getRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    // @TODO: [CAMTRANS] Fill this in...
    REAL lampda = a;
    REAL theta = atan2(v.z,v.x);
    Vector4 pTemp = p;
    pTemp.x /= pTemp.w;
    pTemp.y /= pTemp.w;
    pTemp.z /= pTemp.w;
    pTemp.w = 1;
   if( theta < 0)
    {
        theta += 2*M_PI;
    }
    REAL phi = -atan2(v.y,sqrt(v.x*v.x+v.z*v.z));
    if( phi < 0 )
    {
        phi += 2*M_PI;
    }
    Matrix4x4 m = Matrix4x4::identity();

    m = getTransMat(pTemp)*getInvRotYMat( theta )*getInvRotZMat(phi)*getRotXMat(lampda)*getRotZMat(phi)*getRotYMat( theta )*getInvTransMat(pTemp);
    return m;
}


// @returns the inverse scale matrix described by the vector
Matrix4x4 getInvScaleMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = 1.0/v.x;
    m.data[5] = 1.0/v.y;
    m.data[10] = 1.0/v.z;
    return m;
}

// @returns the inverse translation matrix described by the vector
Matrix4x4 getInvTransMat(const Vector4 &v) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[3] = -v.x;
    m.data[7] = -v.y;
    m.data[11] = -v.z;
    return m;
}

// @returns the inverse rotation matrix about the x axis by the specified angle
Matrix4x4 getInvRotXMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[5] = cos(radians);
    m.data[6] = sin(radians);
    m.data[9] = -sin(radians);
    m.data[10] = cos(radians);
    return m;
}

// @returns the inverse rotation matrix about the y axis by the specified angle
Matrix4x4 getInvRotYMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = cos(radians);
    m.data[2] = -sin(radians);
    m.data[8] = sin(radians);
    m.data[10] = cos(radians);
    return m;
}

// @returns the inverse rotation matrix about the z axis by the specified angle
Matrix4x4 getInvRotZMat (const REAL radians) {

    // @TODO: [CAMTRANS] Fill this in...
     Matrix4x4 m = Matrix4x4::identity();
    m.data[0] = cos(radians);
    m.data[1] = sin(radians);
    m.data[4] = -sin(radians);
    m.data[5] = cos(radians);
    return m;
}

// @returns the inverse rotation matrix around the vector and point by the specified angle
Matrix4x4 getInvRotMat  (const Vector4 &p, const Vector4 &v, const REAL a) {

    // @TODO: [CAMTRANS] Fill this in...
    Matrix4x4 m = getRotMat(p,v,-a);
    return m;
}
//@}---------------------------------------------------------------------
extern bool checkOrthonormal( const Matrix4x4& matrix )
{
    Matrix4x4 result = matrix*(matrix.getTranspose());
    if( EQ(result.a,1) && EQ (result.b,0) && EQ(result.c,0) && EQ(result.d, 0)
            && EQ(result.e, 0) && EQ(result.f,1) && EQ(result.g, 0) && EQ(result.h, 0 )
            && EQ(result.i, 0) && EQ(result.j,0)&&EQ(result.k,1)&&EQ(result.l,0)
            && EQ(result.m, 0)&&EQ(result.n,0)&&EQ(result.o,0)&&EQ(result.p,1)
            )
        return true;
    else
        return false;
}
