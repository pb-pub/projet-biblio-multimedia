#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include "camerawidget.h"

namespace Ui {
class GameWindow;
}

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

private slots:
    void on_pushButton_clicked();

private:
    CameraWidget* cameraWidget;
    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
