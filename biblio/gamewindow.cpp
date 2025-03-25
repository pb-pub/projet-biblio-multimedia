#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"


#include <QVBoxLayout>
#include <qlabel.h>

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    // // Vérifier si cameraWidget a déjà un layout
    // if (!ui->cameraWidgetUI->layout()) {
    //     ui->cameraWidgetUI->setLayout(new QVBoxLayout());
    // }

    // cameraWidget = new CameraWidget(this);
    // ui->cameraWidgetUI->layout()->addWidget(cameraWidget);

    // Vérifier si gameWidget a déjà un layout
    if (!ui->game->layout()) {
        ui->game->setLayout(new QVBoxLayout());
    }
    gameWidget = new GameWidget(this);
    ui->game->layout()->addWidget(gameWidget);




}

GameWindow::~GameWindow()
{
    delete ui;
}

