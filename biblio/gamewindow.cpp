#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"
#include "spherewidget.h"
#include <QVBoxLayout>

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

    // Ajout de SphereWidget dans le widget nommé "game"
    QVBoxLayout *sphereLayout = new QVBoxLayout(ui->game);
    sphereLayout->setContentsMargins(0, 0, 0, 0);
    SphereWidget* sphereWidget = new SphereWidget(ui->game);
    sphereLayout->addWidget(sphereWidget);
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

