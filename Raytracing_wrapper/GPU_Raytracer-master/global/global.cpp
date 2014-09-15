/*!
    @file global.cpp
    @desc: definition of the global variables
    @author: yanli
    @date: May 2013
 */

#include "global.h"
#include "GL/glu.h"

Settings settings;

Settings::Settings(){
}

void Settings::initSettings()
{
    useLighting          = true;
    traceMode            = CPU;
    drawWireFrame        = false;
    drawNormals          = false;
    usePointLights       = true;
    useDirectionalLights = true;
    showAxis             = true;
    showTexture          = true;
    useMultithread       = false;
    useSupersampling     = false;
    useShadow            = false;
    useSpotLights        = false;
    useReflection        = true;
    traceRaycursion      = 4;
    traceThreadNum       = 5;
    showBoundingBox      = false;
    showKdTree           = false;
    useKdTree            = true;
}

/**
 * @brief getGLErrorString: get GL error string
 * @return: the error string
 */
const char* getGLErrorString()
{
    GLenum err;
    if( (err = glGetError()) != GL_NO_ERROR )
    {
        return (const char*)gluErrorString( err );
    }
    else
        return NULL;

}
