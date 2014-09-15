/*!
    @file mainwindow.cpp
    @desc: definition of MainWindow class
    @author: yanli
    @date: Dec 2013
 */
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "scene.h"
#include "CPUrayscene.h"

extern Settings settings;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Set the default widget to be view3D
    activateView3D();
    settings.initSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    // Not used
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Scene",
                                                    "",
                                                    "Scene Files (*.xml)");
    ui->view3D->loadScene(fileName);
    ui->view3D->createGPUScene();
}

void MainWindow::on_radioButtonCPUTrace_clicked()
{
    settings.traceMode = CPU;
    ui->view3D->activateGPUtrace(false);
}

void MainWindow::on_radioButtonGPUTrace_clicked()
{
    settings.traceMode = GPU;
}

void MainWindow::activateView3D()
{
    ui->tabWidget->setCurrentWidget(ui->tab_1);
}

void MainWindow::activateView2D()
{
    ui->tabWidget->setCurrentWidget(ui->tab_2);
}

void MainWindow::on_checkBox_spot_light_toggled(bool checked)
{
    settings.useSpotLights = checked;

    // Update lights
    ui->view3D->getScene()->setLights();
    ui->view3D->syncGPUGlobalSetting();
}

void MainWindow::on_checkBox_shadow_toggled(bool checked)
{
    settings.useShadow = checked;
    ui->view3D->syncGPUGlobalSetting();
}

void MainWindow::on_checkBox_multi_thread_toggled(bool checked)
{
    settings.useMultithread = checked;
}

void MainWindow::on_checkBox_dir_light_toggled(bool checked)
{
    settings.useDirectionalLights = checked;
    // Update lights
    ui->view3D->getScene()->setLights();
    ui->view3D->syncGPUGlobalSetting();
}

void MainWindow::on_checkBox_point_light_toggled(bool checked)
{
    settings.usePointLights = checked;
    // Update lights
    ui->view3D->getScene()->setLights();
    ui->view3D->syncGPUGlobalSetting();
}

void MainWindow::on_checkBox_supersampling_toggled(bool checked)
{
    settings.useSupersampling = checked;
    ui->view3D->syncGPUGlobalSetting();
}

void MainWindow::on_stopButton_clicked()
{
      ui->view3D->activateGPUtrace(false);
}

void MainWindow::on_traceButton_clicked()
{
    Scene* curScene = ui->view3D->getScene();

    if (curScene && settings.traceMode == CPU)
    {
        CPURayScene* newScene = new CPURayScene(curScene);
        ui->view2D->setScene(newScene);
        ui->view2D->traceScene(ui->view3D->getCamera(),
                               ui->view2D->size().width(),
                               ui->view2D->size().height());
        if (ui->radioButtonCPUTrace->isChecked())
            activateView2D();
    }
    else
    {
        ui->view3D->activateGPUtrace(!ui->view3D->isActivatedGPUtrace());
    }
}

void MainWindow::on_checkBox_use_kdtree_toggled(bool checked)
{
    settings.useKdTree = checked;
    ui->view3D->syncGPUGlobalSetting();
}
