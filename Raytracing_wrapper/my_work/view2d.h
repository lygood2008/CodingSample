/*!
    @file view2D.h
    @desc: declarations of View2D class
    @author: yanli
    @date: May 2013
 */

#ifndef VIEW2D_H
#define VIEW2D_H

#include <QWidget>
#include "global.h"

class Scene;
class CPURayScene;
class OrbitCamera;

/**
 * @class: View2D
 * @brief The View2D class is the class for 2D canvas
 */
class View2D :
        public QWidget
{
    Q_OBJECT

public:

    View2D(QWidget *parent);
    ~View2D();

    BGRA *data() { return (BGRA *)m_image->bits(); }

    /**
     * @brief resize: resize
     * @param width
     * @param height
     */
    void resize(int width, int height);

    /**
     * @brief loadImage: load an image from file
     * @param file: file path
     * @return: success or failure
     */
    bool loadImage(const QString &file);

    /**
     * @brief saveImage: save an image into file
     * @param file: file path
     * @return: success or failure
     */
    bool saveImage(const QString &file);

    /**
     * @brief newImage: create a new image
     */
    void newImage();

    /**
     * @brief saveImage: save the current image
     * @return: success or not
     */
    bool saveImage();
    
    /**
     * @brief setScene: set my scene (must delete original one)
     * @param scene: the given scene
     */
    void setScene(CPURayScene* scene);

    /**
     * @brief traceScene: do CPU ray tracing
     * @param camera: orbit camera
     * @param width: width of canvas
     * @param height: height of canvas
     */
    void traceScene(OrbitCamera* camera, int width, int height);

protected:

    /**
     * @brief paintEvent:handles paint event
     */
    virtual void paintEvent(QPaintEvent *);

    QImage *m_image; // image for the current canvas

private:

    CPURayScene* m_scene; // My CPU ray scene
};

#endif // VIEW2D_H
