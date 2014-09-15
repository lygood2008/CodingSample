/*!
    @file trace.cpp
    @desc: the definitions of wrapper functions for doing ray tracing
    @author: yanli
    @date: May 2013
 */

#include "trace.h"
#include "intersect.h"
#include "utils.h"
#include "global.h"
#include "pos_check.h"

#include "cone_intersect.h"
#include "cube_intersect.h"
#include "sphere_intersect.h"
#include "cylinder_intersect.h"

void doRayTrace(BGRA* data,
                const int width,
                const int height,
                const int beginIndex,
                const int endIndex,
                const CS123SceneGlobalData& global,
                QVector<SceneObject>& objects,
                const QList<CS123SceneLightData>& lights,
                const Vector4& eyePos,
                const float near,
                const Matrix4x4& invViewTransMat,
                KdTree* tree,
                AABB extends)
{

    assert(beginIndex >= 0 && beginIndex <= endIndex);
    assert(endIndex <= width*height);

    for (int i = beginIndex; i < endIndex; i++)
    {
        int row = i / width;
        int col = i - row * width;

        Vector3 sumColor(0.f, 0.f, 0.f);

        REAL weight = 1.f;
        Vector2 poses[5];
        poses[0] = Vector2(col, row);
        int size = 1;
        if (settings.useSupersampling)
        {
            poses[1] = Vector2(col - 0.5, row - 0.5);
            poses[2] = Vector2(col - 0.5, row + 0.5);
            poses[3] = Vector2(col + 0.5, row - 0.5);
            poses[4] = Vector2(col + 0.5, row + 0.5);
            weight = 1.f/5;
            size = 5;
        }

        int k = 0;
        do
        {
            Vector4 pFilmCam(((REAL)(2 * poses[k].x)) / width - 1,
                             1 - ((REAL)(2 * poses[k].y)) / height,
                             -1,
                             1);
            CS123SceneColor color;

            Vector4 pFilmWorld = invViewTransMat*pFilmCam;
            Vector4 d          = pFilmWorld - eyePos;

            d = d.getNormalized();
            Vector4 eyePosNear = eyePos + d * near;

            color = recursiveTrace(eyePosNear,
                                   d,
                                   global,
                                   objects,
                                   lights,
                                   tree,
                                   extends,
                                   -1,
                                   settings.traceRaycursion);

            sumColor.x += weight * color.r;
            sumColor.y += weight * color.g;
            sumColor.z += weight * color.b;

            k++;

        }while (k < size);

        sumColor = sumColor * 255;

        mclamp(sumColor.x, 0.f, 255.f);data[i].r = sumColor.x;
        mclamp(sumColor.y, 0.f, 255.f);data[i].g = sumColor.y;
        mclamp(sumColor.z, 0.f, 255.f);data[i].b = sumColor.z;
    }
}

CS123SceneColor recursiveTrace(const Vector4& pos,
                               const Vector4& d,
                               const CS123SceneGlobalData& global,
                               QVector<SceneObject>& objects,
                               const QList<CS123SceneLightData>& lights,
                               KdTree* tree,
                               AABB extends,
                               int curIndex,
                               int count)
{

    CS123SceneColor result;
    // if count is less than 0,  it's maximum recursion
    if (count <= 0)
        return result;

    count--;

    Vector3 norm;
    int objectIndex = -1;
    int faceIndex = -1;
    CS123SceneColor texColor;
    REAL t = intersect(pos, objects, d, objectIndex, faceIndex, tree, extends);
    // if t > 0, then compute the intersect point and blend the color
    if (t > 0)
    {
        if (t == 1)
         {
            CS123SceneColor r(1.f);
            return r;
        }

        Vector4 intersectPoint = pos + t * d;
        Vector4 eyeSpaceIntersectPoint =
                objects[objectIndex].m_invTransform*pos +
                t*objects[objectIndex].m_invTransform*d;

        switch (objects[objectIndex].m_primitive.type)
        {
        case PRIMITIVE_CUBE:
            norm = getCubeNorm(faceIndex);
            break;
        case PRIMITIVE_CYLINDER:
            norm = getCylinderNorm(eyeSpaceIntersectPoint, faceIndex);
            break;
        case PRIMITIVE_CONE:
            norm = getConeNorm(eyeSpaceIntersectPoint, faceIndex);
            break;
        case PRIMITIVE_SPHERE:
            norm = getSphereNorm(eyeSpaceIntersectPoint);
            break;
        case PRIMITIVE_MESH:
            break;
        case PRIMITIVE_TORUS:
            break;
        default:
            assert(0);
            break;
        }

        if (objects[objectIndex].m_texture.m_textureHandle &&
           settings.showTexture &&
           objects[objectIndex].m_texture.m_texPointer)
        {
            switch (objects[objectIndex].m_primitive.type)
            {
            case PRIMITIVE_CUBE:
                texColor = getCubeIntersectTexColor(objects[objectIndex],
                                                    faceIndex,
                                                    eyeSpaceIntersectPoint);
                        break;
            case PRIMITIVE_CYLINDER:
                texColor = getCylinderIntersectTexColor(objects[objectIndex],
                                                        faceIndex,
                                                        eyeSpaceIntersectPoint);
                        break;
            case PRIMITIVE_CONE:
                texColor = getConeIntersectTexColor(objects[objectIndex],
                                                    faceIndex,
                                                    eyeSpaceIntersectPoint);
                        break;
            case PRIMITIVE_SPHERE:
                texColor = getSphereIntersectTexColor(objects[objectIndex],
                                                      eyeSpaceIntersectPoint);
                        break;
            case PRIMITIVE_MESH:
                        break;
            case PRIMITIVE_TORUS:
                        break;
            default:assert(0);
                        break;
            }
        }

        CS123SceneColor colorNormal;
        CS123SceneColor colorReflection;
        CS123SceneColor colorRefraction;

        Vector4 tempNorm = Vector4(norm.x, norm.y, norm.z, 0);
        tempNorm = objects[objectIndex].m_invTTransformWithoutTrans * tempNorm;

        // nomalize the new norm
        norm = Vector3(tempNorm.x, tempNorm.y, tempNorm.z).unit();

        colorNormal = computeObjectColor(objectIndex,objects,
                                         global,
                                         lights,
                                         tree,
                                         extends,
                                         intersectPoint,
                                         norm,
                                         pos,
                                         texColor);

        // if refecltion is enabled then do recursive retracing
        if (settings.useReflection )
        {
            REAL projection = -(d.x * norm.x + d.y * norm.y + d.z * norm.z);
            bool zeroReflection =
                    EQ4(objects[objectIndex].m_primitive.material.cReflective.a,
                        objects[objectIndex].m_primitive.material.cReflective.r,
                        objects[objectIndex].m_primitive.material.cReflective.g,
                        objects[objectIndex].m_primitive.material.cReflective.b,
                        0);

            if (projection > 0 && global.ks > 0 && !zeroReflection)
            {
                Vector4 reflection = getReflectionDir(norm, d);

                intersectPoint +=
                        Vector4(reflection.x, reflection.y, reflection.z, 0) *
                        EPSILON;

                colorReflection = recursiveTrace(intersectPoint,
                                                 reflection,
                                                 global,
                                                 objects,
                                                 lights,
                                                 tree,
                                                 extends,
                                                 curIndex,
                                                 count);
                colorReflection *=
                        objects[objectIndex].m_primitive.material.cReflective *
                        global.ks;
            }


            bool zeroRefraction =
                    EQ4(objects[objectIndex].m_primitive.material.cTransparent.a,
                        objects[objectIndex].m_primitive.material.cTransparent.r,
                        objects[objectIndex].m_primitive.material.cTransparent.g,
                        objects[objectIndex].m_primitive.material.cTransparent.b,
                        0);

            if (!zeroRefraction)
            {
                // Refracetion part
                float n1 = 0, n2 = 0;
                if (curIndex != -1)
                {
                    // The ray may be inside an object
                    n1 = objects[curIndex].m_primitive.material.ior;
                    Vector3 normFace;
                    // bump the start point to be a little bit
                    if (d.x*norm.x + d.y*norm.y + d.z*norm.z > 0)
                        normFace = Vector3(-norm.x, -norm.y, -norm.z);
                    else
                        normFace = norm;

                    Vector4 bumpPos =
                            intersectPoint + Vector4(-normFace.x, -normFace.y,
                                                     -normFace.z, 0) * EPSILON * 2;

                    QMap<int, int> indexMap;
                    QVector<SceneObject> list = checkPos(objects, bumpPos,
                                                         indexMap);
                    int dummyObjectIndex = -1, dummyFaceindex = -1;

                    REAL t2 = -1;
                    if (list.size() != 0)
                        t2 = intersect(bumpPos, list,
                                       Vector4(-normFace.x, -normFace.y,
                                               -normFace.z, 0),
                                       dummyObjectIndex, dummyFaceindex,
                                       tree, extends, true);
                    if (t2 > 0)
                    {
                        n2 = list[dummyObjectIndex].m_primitive.material.ior;
                        curIndex = indexMap[dummyObjectIndex];
                    }
                    else
                    {
                        // The ray is towards air
                        n2 = 1;
                        curIndex = -1;
                    }
                }
                else
                {
                    // If curIndex == -1, then the ray is from air
                    n1 = 1;
                    n2 = objects[objectIndex].m_primitive.material.ior;
                    curIndex = objectIndex;
                }

                Vector4 refraction;
                // Check the angle between incident ray and norm
                if (d.x*norm.x + d.y*norm.y + d.z*norm.z > 0)
                    refraction = getRefracetionDir(-norm, d, n1, n2);
                else
                    refraction = getRefracetionDir(norm, d, n1, n2);
                if (refraction != Vector4::zero())
                {
                    intersectPoint += refraction * EPSILON * 2;
                    intersectPoint.w = 1;
                    colorRefraction = recursiveTrace(intersectPoint,
                                                      refraction,
                                                      global,
                                                      objects,
                                                      lights,
                                                      tree,
                                                      extends,
                                                      curIndex,
                                                      count
                                                     );
                    colorRefraction *=
                         objects[objectIndex].m_primitive.material.cTransparent*
                         global.ks;
                }
            }
        }
        result = colorNormal + colorReflection + colorRefraction;
    }
    return result;
}

CS123SceneColor computeObjectColor(const int& objectIndex,
                                   QVector<SceneObject>& objects,
                                   const CS123SceneGlobalData& global,
                                   const QList<CS123SceneLightData>& lights,
                                   KdTree* tree,
                                   AABB extends,
                                   const Vector4& pos,
                                   const Vector3& norm,
                                   const Vector4& eyePos,
                                   const CS123SceneColor& texture)
{

    SceneObject object = objects[objectIndex];
    CS123SceneColor ambient = object.m_primitive.material.cAmbient;
    ambient *= global.ka;
    ambient.a = 0;

    // lightSum contains the sum of the diffuse color, ambient color(hack),
    // specular color, and reflective color(recursive)
    CS123SceneColor lightSum;

    for (int i = 0; i < lights.size(); i++)
    {
        CS123SceneLightData currentLight = lights[i];
        Vector4 lightDir = Vector4(0, 1, 0, 0);

        bool unapplicable = false;

        REAL attenuation = 1;
        if (currentLight.type != LIGHT_DIRECTIONAL)
        {
            // compute the attenuation
            REAL dLight = sqrt(SQ(currentLight.pos.x - pos.x) +
                               SQ(currentLight.pos.y - pos.y) +
                               SQ(currentLight.pos.z - pos.z));
            attenuation = MIN(1.0 / (currentLight.function.x +
                                     currentLight.function.y * dLight +
                                     currentLight.function.z * SQ(dLight)), 1);
        }

        CS123SceneColor lightIntensity = currentLight.color;
        switch (currentLight.type)
        {
        case LIGHT_POINT:
        {
            if (settings.usePointLights)
            {
                lightDir = (currentLight.pos - pos).getNormalized();
            }
            else
            {
                unapplicable = true;
            }
            break;
        }
        case LIGHT_DIRECTIONAL:
        {
            if (settings.useDirectionalLights)
            {
                lightDir = -currentLight.dir.getNormalized();
            }
            else
            {
                unapplicable = true;
            }
            break;
        }
        case LIGHT_SPOT:
        {
            if (settings.useSpotLights)
            {
                lightDir = (currentLight.pos - pos).getNormalized();
                Vector4 majorDir = -currentLight.dir.getNormalized();

                REAL lightRadians  = currentLight.penumbra / 180.0 * M_PI;
                REAL spotIntensity = lightDir.dot(majorDir);

                // the object is not in the cone
                if (spotIntensity < cos(lightRadians))
                {
                    lightIntensity = CS123SceneColor(0);
                }
                else
                {
                    REAL temp = pow(spotIntensity, 5);
                    lightIntensity *= temp;
                    lightIntensity.a = 0;
                }
            }
            else
            {

                unapplicable = true;
            }
            break;
        }
        case LIGHT_AREA:
        {
            unapplicable = true;
            break;
        }
        default:
        {
            unapplicable = true;
            break;
        }
        }
        if (unapplicable)
            continue;

        REAL dotLN =
                lightDir.x * norm.x + lightDir.y * norm.y + lightDir.z * norm.z;
        // in intersect assignment, this is used for checking if the object is
        // in shadow
        if (dotLN < 0.0)
            dotLN = 0.0;

       // Check if the object is in shadow of light
        if (settings.useShadow)
        {
            int objectIndex2 = -1;
            int faceIndex    = -1;
            Vector4 lightPos;

            if (currentLight.type != LIGHT_DIRECTIONAL)
            {
                lightPos = currentLight.pos;
                intersect(lightPos, objects, -lightDir, objectIndex2, faceIndex,
                          tree, extends);
            }
            else
            {
                // For directional light we use a dummy position
                intersect(pos + Vector4(norm.x, norm.y, norm.z, 0) * EPSILON,
                          objects, (lightDir), objectIndex2, faceIndex, tree,
                          extends);
            }

            if (objectIndex2 != objectIndex && objectIndex2 != -1)
                continue;
        }

        // compute diffuse light color
        // if using texture mapping, then blend the diffuse with diffuse color
        if (settings.showTexture && object.m_texture.m_textureHandle &&
            object.m_texture.m_texPointer)
        {
            lightSum +=
                    attenuation * lightIntensity * dotLN *
                    (global.kd *((object.m_primitive.material.cDiffuse * texture)));
        }
        else
        {
            lightSum +=
                    attenuation * lightIntensity * dotLN *
                    (global.kd *((object.m_primitive.material.cDiffuse)));
        }

        Vector4 reflection = getReflectionDir(norm, -lightDir);
        Vector4 sight = eyePos - pos;
        sight = sight.getNormalized();

        REAL dotEN  =
                sight.x * reflection.x + sight.y * reflection.y +
                sight.z * reflection.z;

        if (dotEN < 0.0)
            dotEN = 0.0;

        dotEN = pow(dotEN, object.m_primitive.material.shininess);

        lightSum +=
                attenuation * lightIntensity * dotEN * global.ks *
                object.m_primitive.material.cSpecular;
    }

    lightSum += ambient;
    lightSum.a = 0;

    mclamp(lightSum.r, 0.f, 1.f);
    mclamp(lightSum.g, 0.f, 1.f);
    mclamp(lightSum.b, 0.f, 1.f);

    return lightSum;
}
