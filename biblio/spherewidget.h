#ifndef SPHEREWIDGET_H
#define SPHEREWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <GL/glu.h>
#include <QTimer>

class SphereWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum FruitType {
        APPLE,
        ORANGE,
        BANANA,
        PEAR,
        BOMB
    };

    explicit SphereWidget(QWidget *parent = nullptr);
    ~SphereWidget();
    
    void setFruitType(FruitType type);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    GLUquadric* quadric;
    FruitType currentFruit;
    QTimer* bombTimer;
    float bombFlashIntensity;
    bool bombFlashIncreasing;
    
    void drawApple();
    void drawOrange();
    void drawBanana();
    void drawPear();
    void drawBomb();
    
private slots:
    void updateBombFlash();
};

#endif // SPHEREWIDGET_H
