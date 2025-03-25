#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include "camerawidget.h"
#include "gamewidget.h"

namespace Ui {
class GameWindow;
}

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();


private:
    CameraWidget* cameraWidget;
    GameWidget* gameWidget;
    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
