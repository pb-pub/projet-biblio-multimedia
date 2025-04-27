#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "camerawidget.h"
#include "mainwindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    if (!ui->camera->layout()) {
        ui->camera->setLayout(new QVBoxLayout());
    }

    cameraWidget = new CameraWidget(this);
    ui->camera->layout()->addWidget(cameraWidget);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::on_pushButton_clicked()
{
    this->close();
    this->deleteLater(); 
    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

}

