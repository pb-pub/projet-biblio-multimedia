#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);


    // Vérifier si cameraWidget a déjà un layout
    if (!ui->cameraWidgetUI->layout()) {
        ui->cameraWidgetUI->setLayout(new QVBoxLayout());
    }


    cameraWidget = new CameraWidget(this);
    ui->cameraWidgetUI->layout()->addWidget(cameraWidget);

}

GameWindow::~GameWindow()
{
    delete ui;
}

void GameWindow::on_pushButton_clicked()
{
    this->close();
    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();
}

