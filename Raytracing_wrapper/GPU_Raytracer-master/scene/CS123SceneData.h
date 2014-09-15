/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
/*!
   @file   CS123SceneData.h
   @author Nong Li
   @date   Fall 2008

   @brief  Header file containing scene data structures.
*/

#ifndef __CS123_SCENE_DATA__
#define __CS123_SCENE_DATA__

/* Includes */
#include "CS123Algebra.h"
#include "vector.h"
#include <vector>
#include <string>
using namespace std;

//! Enumeration for light types.
enum LightType {
   LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT, LIGHT_AREA
};

//! Enumeration for types of primitives that can be stored in a scene file.
enum PrimitiveType {
   PRIMITIVE_NONE = 0,PRIMITIVE_CUBE = 1, PRIMITIVE_CONE = 2, PRIMITIVE_CYLINDER = 3,
   PRIMITIVE_TORUS= 4, PRIMITIVE_SPHERE = 5,PRIMITIVE_MESH
};

//! Enumeration for types of transformations that can be applied to objects, lights, and cameras.
enum TransformationType {
   TRANSFORMATION_TRANSLATE, TRANSFORMATION_SCALE, 
   TRANSFORMATION_ROTATE, TRANSFORMATION_MATRIX
};

//! Struct to store a RGBA color in floats [0,1]
struct CS123SceneColor 
{
    union {
        struct {
           float r;
           float g;
           float b;
           float a;
        };
        float channels[4]; // points to the same four floats above...
    };
    CS123SceneColor()
    {
        r = 0;
        g = 0;
        b = 0;
        a = 0;
    }

    CS123SceneColor(float f)
    {
        a = r = g = b = f;
    }

    CS123SceneColor(float cr, float cg, float cb, float ca)
    {
        r = cr;
        g = cg;
        b = cb;
        a = ca;
    }

    CS123SceneColor operator + (const CS123SceneColor &c) const { return CS123SceneColor(r + c.r, g + c.g, b + c.b, a + c.a); }
    CS123SceneColor operator - (const CS123SceneColor &c) const { return CS123SceneColor(r - c.r, g - c.g, b - c.b, a - c.a); }
    CS123SceneColor operator * (const CS123SceneColor &c) const { return CS123SceneColor(r * c.r, g * c.g, b * c.b, a / c.a); }
    CS123SceneColor operator / (const CS123SceneColor &c) const { return CS123SceneColor(r / c.r, g / c.g, b / c.b, a / c.a); }
    CS123SceneColor operator + (float s) const { return CS123SceneColor(r + s, g + s, b + s, a + s); }
    CS123SceneColor operator - (float s) const { return CS123SceneColor(r - s, g - s, b - s, a - s); }
    CS123SceneColor operator * (float s) const { return CS123SceneColor(r * s, g * s, b * s, a * s); }
    CS123SceneColor operator / (float s) const { return CS123SceneColor(r / s, g / s, b / s, a / s); }

    friend CS123SceneColor operator + (float s, const CS123SceneColor &c) { return CS123SceneColor(s + c.r, s + c.g, s + c.b, s + c.a); }
    friend CS123SceneColor operator - (float s, const CS123SceneColor &c) { return CS123SceneColor(s - c.r, s - c.g, s - c.b, s - c.a); }
    friend CS123SceneColor operator * (float s, const CS123SceneColor &c) { return CS123SceneColor(s * c.r, s * c.g, s * c.b, s * c.a); }
    friend CS123SceneColor operator / (float s, const CS123SceneColor &c) { return CS123SceneColor(s / c.r, s / c.g, s / c.b, s / c.a); }

    CS123SceneColor &operator += (const CS123SceneColor &c) { return *this = *this + c; }
    CS123SceneColor &operator -= (const CS123SceneColor &c) { return *this = *this - c; }
    CS123SceneColor &operator *= (const CS123SceneColor &c) { return *this = *this * c; }
    CS123SceneColor &operator /= (const CS123SceneColor &c) { return *this = *this / c; }
    CS123SceneColor &operator += (float s) { return *this = *this + s; }
    CS123SceneColor &operator -= (float s) { return *this = *this - s; }
    CS123SceneColor &operator *= (float s) { return *this = *this * s; }
    CS123SceneColor &operator /= (float s) { return *this = *this / s; }

};

typedef CS123SceneColor RGBA;

//! Scene global color coefficients
struct CS123SceneGlobalData 
{
   float ka;  //! global ambient coefficient
   float kd;  //! global diffuse coefficient
   float ks;  //! global specular coefficient
   float kt;  //! global transparency coefficient
};

//! Data for a single light
struct CS123SceneLightData 
{
   int id;
   LightType type;

   CS123SceneColor color;
   Vector3 function;    //! Attenuation function

   Vector4  pos;        //! Not applicable to directional lights
   Vector4 dir;         //! Not applicable to point lights

   float radius;        //! Only applicable to spot lights
   float penumbra;      //! Only applicable to spot lights
   float angle;         //! Only applicable to spot lights

   float width, height; //! Only applicable to area lights
};

//! Data for scene camera
struct CS123SceneCameraData
{
   Vector4 pos;
   Vector4 look;
   Vector4 up;

   float heightAngle;
   float aspectRatio;

   float aperture;      //! Only applicable for depth of field
   float focalLength;   //! Only applicable for depth of field
};

//! Data for file maps (ie: texture maps)
struct CS123SceneFileMap
{
   bool isUsed;
   string filename;
   float repeatU;
   float repeatV;
};

//! Data for scene materials
struct CS123SceneMaterial 
{
   /*! @brief This field specifies the diffuse color of the object. 
   This is the color you need to use for the object in sceneview. 
   You can get away with ignoring the other color values until 
   intersect and ray. */ 
   CS123SceneColor cDiffuse;
   
   CS123SceneColor cAmbient;
   CS123SceneColor cReflective;
   CS123SceneColor cSpecular;
   CS123SceneColor cTransparent;
   CS123SceneColor cEmissive;

   CS123SceneFileMap* textureMap;
   float blend;

   CS123SceneFileMap* bumpMap;

   float shininess;

   float ior;           //! index of refaction
};

//! Data for a single primitive.
struct CS123ScenePrimitive
{
   PrimitiveType type;
   string meshfile;     //! Only applicable to meshes
   CS123SceneMaterial material;
};

/*!

@struct CS123SceneTransformation
@brief Data for transforming a scene object.

Aside from the TransformationType, the remaining of the data in the
struct is mutually exclusive

*/
struct CS123SceneTransformation
{
   TransformationType type; /*!< Specify whether a given transformation is a rotation, a translation, a scale, or a custom matrix. */
   Vector4 translate;       /*!< The translation ctor for the transformation. Only valid if the transformation is a translation. */
   Vector4 scale;           /*!< The scale ctor for the transformation. Only valid if the transformation is a scale. */
   Vector4 rotate;          /*!< The axis of rotation. Only valid if the transformation is a rotation. */
   float angle;             /*!< The rotation angle in **radians**. Only valid if the transformation is a rotation. */
   Matrix4x4 matrix;        /*!< The matrix for the transformation. Only valid if the transformation is a custom matrix. */
};

//! Structure for non-primitive scene objects
struct CS123SceneNode
{
   /*! Transformation at this node */
   std::vector<CS123SceneTransformation*> transformations;

   /*! Primitives at this node */
   std::vector<CS123ScenePrimitive*> primitives;

   /*! Children of this node */
   std::vector<CS123SceneNode*> children;
};
#endif

