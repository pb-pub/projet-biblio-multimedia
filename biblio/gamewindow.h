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
    int lives = 3;
    int score = 0;
    CameraWidget* cameraWidget;
    GameWidget* gameWidget;
    Ui::GameWindow *ui;

    void updateLabelDisplay();
};

#endif // GAMEWINDOW_H
