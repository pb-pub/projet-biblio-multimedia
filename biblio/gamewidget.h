#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include "fruit.h"
#include <qlabel.h>
#include <vector>

namespace Ui {
class GameWidget;
}

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();
    
    void updateFruitDisplay();

public slots:
    // Change these from private to public slots to make them accessible via signal/slot
    void initializeGL();
    void initializeTextures();
    void startCountdown(int seconds);
    void resizeGL(int width, int height);
    void paintGL();

private:
    Ui::GameWidget *ui;
    std::vector<Fruit*> m_fruit;
    GLuint* textures;
    QFont m_font;   
    QLabel* label;
    GLUquadric* cylinder;
};

#endif // GAMEWIDGET_H
