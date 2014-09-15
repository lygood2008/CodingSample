/*!
    @file view2D.cpp
    @desc: definitions of View2D class
    @author: yanli
    @date: May 2013
 */


#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QString>

#include "view2d.h"
#include "CPUrayscene.h"

View2D::View2D(QWidget *parent) :
    QWidget(parent)
{

    COMPILE_TIME_ASSERT(sizeof(BGRA) == 4);

    m_image = NULL;
    m_scene = NULL;

    resize(WIN_WIDTH, WIN_HEIGHT);
    setFixedSize(WIN_WIDTH, WIN_HEIGHT);
}

View2D::~View2D()
{
    if(m_image)
        delete m_image;

    if(m_scene)
        delete m_scene;
}

void View2D::resize(int width, int height)
{
    // clamp negative sizes so we always have at least one pixel
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    delete m_image;
    m_image = new QImage(width, height, QImage::Format_RGB32);

    // set the new image to black
    memset(m_image->bits(), 0, width * height * sizeof(BGRA));

    setFixedSize(width, height);
    update();
}

bool View2D::loadImage(const QString &file)
{
    QImage *temp = new QImage();

    if (!temp->load(file))
        return false;

    // make sure the image is RGB (not monochrome, for example)
    if (temp->format() != QImage::Format_RGB32)
    {
        QImage *old = temp;
        temp = new QImage(old->convertToFormat(QImage::Format_RGB32));
        delete old;
    }

    // show the new image
    resize(temp->width(), temp->height());
    memcpy(data(), temp->bits(), temp->byteCount());
    update();

    delete temp;
    return true;
}

bool View2D::saveImage(const QString &file)
{
    return m_image->save(file, NULL, -1);
}

void View2D::newImage()
{
    resize(500, 500);
}

bool View2D::saveImage()
{
    QString file = QFileDialog::getSaveFileName(this);
    if (!file.isNull())
    {
        // what format to save in, so make sure it gets one
        if (!file.contains('.'))
            file += ".png";

        if (saveImage(file))
            return true;
        QMessageBox::critical(this, "Error", "Could not save image \"" +
                              file + "\"");
    }
    return false;
}

void View2D::setScene(CPURayScene* scene)
{
    if(scene == NULL)
        std::cerr<<"You set an empty scene\n"<<endl;

    // Delete old scene
    if(m_scene)
        delete m_scene;

    m_scene = scene;
}

void View2D::traceScene(OrbitCamera* camera, int width, int height)
{
    if(m_scene)
    {
        m_scene->traceScene(this, camera, width, height);
    }
}

void View2D::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), *m_image);
}
