#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamewindow.h"
#include "settingswindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_2_clicked()
{
    this->close(); 
    GameWindow* gameWindow = new GameWindow();
    gameWindow->show();
}


void MainWindow::on_pushButton_clicked()
{
    this->close();
    SettingsWindow* settingsWindow = new SettingsWindow();
    settingsWindow->show();
}

