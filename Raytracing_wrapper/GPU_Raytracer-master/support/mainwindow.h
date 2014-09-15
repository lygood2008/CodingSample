/*!
    @file mainwindow.h
    @desc: MainWindow is responsible for handling callbacks from UI
    @author: yanli
    @date: May 2013
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

/*!
   @class: MainWindow
   @brief: The MainWindow class is mainly used for handling callbacks.
           It extends QMainWindow
 */
class MainWindow :
        public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void activateView3D();
    void activateView2D();

private slots:

    /**
     * Callback functions
     */
    void on_actionExit_triggered();
    void on_actionOpen_triggered();
    void on_radioButtonCPUTrace_clicked();
    void on_radioButtonGPUTrace_clicked();
    void on_checkBox_spot_light_toggled(bool checked);
    void on_checkBox_shadow_toggled(bool checked);
    void on_checkBox_multi_thread_toggled(bool checked);
    void on_checkBox_dir_light_toggled(bool checked);
    void on_checkBox_point_light_toggled(bool checked);
    void on_checkBox_supersampling_toggled(bool checked);
    void on_stopButton_clicked();
    void on_traceButton_clicked();
    void on_checkBox_use_kdtree_toggled(bool checked);
};

#endif // MAINWINDOW_H

