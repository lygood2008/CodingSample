/*!
    @file CPURayScene.cpp
    @desc: definitions of CPURayScene class
    @author: yanli
    @date: May 2013
 */

#include "CPUrayscene.h"
#include "camera.h"
#include "view2d.h"
#include "trace.h"
#include "trace_thread.h"

CPURayScene::CPURayScene()
{

}

CPURayScene::CPURayScene(Scene* scene)
{

    m_globalData = scene->getGlobal();
    m_lightData  = scene->getLight();
    m_objects    = scene->getObjects();
    m_tree       = scene->getKdTree();
    m_extends    = scene->getExtends();
}

CPURayScene::~CPURayScene()
{

}

void CPURayScene::traceScene(View2D* view2D,
                             OrbitCamera* camera,
                             int width,
                             int height)
{

    assert(view2D);
    assert(camera);
    assert(width > 0 && height > 0);

    camera->updateMatrices();
    Vector4 eyePos;
    Matrix4x4 invViewTransMat;

    // Resize the view2D
    view2D->resize(width, height);

    eyePos = camera->getEyePos();

    invViewTransMat = camera->getInvViewTransMatrix();

    BGRA* pixels = view2D->data();

    if (settings.useMultithread)
    {
        assert(settings.traceThreadNum > 0);
        int unitLength = width * height / settings.traceThreadNum;
        TraceThread* threads = new TraceThread[settings.traceThreadNum];

        for (int i = 0; i < settings.traceThreadNum - 1; i++)
            threads[i].pack(pixels,
                            width,
                            height,
                            i*unitLength,
                            (i + 1) * unitLength,
                            m_globalData,
                            m_objects,
                            m_lightData,
                            eyePos,
                            camera->getNear(),
                            invViewTransMat,
                            m_tree,
                            m_extends);

        threads[settings.traceThreadNum-1].pack(pixels,
                                                width,
                                                height,
                                                (settings.traceThreadNum - 1) *
                                                unitLength,
                                                width * height,
                                                m_globalData,
                                                m_objects,
                                                m_lightData,
                                                eyePos,
                                                camera->getNear(),
                                                invViewTransMat,
                                                m_tree,
                                                m_extends);

        for (int i = 0; i < settings.traceThreadNum; i++)
            threads[i].start();

        for (int i = 0; i < settings.traceThreadNum; i++)
            threads[i].wait();

        delete []threads;
    }
    else
    {
        doRayTrace(pixels,
                   width,
                   height,
                   0,
                   width*height,
                   m_globalData,
                   m_objects,
                   m_lightData,
                   eyePos,
                   camera->getNear(),
                   invViewTransMat,
                   m_tree,
                   m_extends);
    }
}

