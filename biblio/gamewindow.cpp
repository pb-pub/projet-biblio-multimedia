#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);
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

