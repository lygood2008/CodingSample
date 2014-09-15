/*!
    @file main.cpp
    @desc: main function
    @author: yanli
    @date: May 2013
 */

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    bool startFullscreen = false;

    w.show();

    if (startFullscreen) {
        w.setWindowState(w.windowState() | Qt::WindowFullScreen);
    }

    return a.exec();
}

