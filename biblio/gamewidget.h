#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include "fruit.h"
#include <qlabel.h>
#include <vector>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include "camerahandler.h"
#include "cannon.h" // Add this include

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

signals:
    void scoreIncreased(); // Signal emitted when a fruit is hit
    void lifeDecrease(); // Signal emitted when a bomb is hit

public slots:
    // Change these from private to public slots to make them accessible via signal/slot
    void initializeGL();
    void initializeTextures();
    void startCountdown(int seconds);
    void resizeGL(int width, int height);
    void paintGL();
    void updateFrame(); // For camera updates

private:
    Ui::GameWidget *ui;
    std::vector<Fruit*> m_fruit;
    GLuint* textures;
    QFont m_font;   
    QLabel* label;
    GLUquadric* cylinder;
    CameraHandler* cameraHandler;
    QTimer *cameraTimer;
    cv::Mat currentFrame;
    cv::Mat grayFrame;
    bool cameraInitialized = false;
    QVector3D projectedPoint;
    bool hasProjectedPoint = false;
    Cannon cannon; // Add the cannon member
    bool displayCamera = false;
    GLuint m_cameraTextureId; // Texture ID for camera feed

    void keyPressEvent(QKeyEvent *event);
    
    // Helper method to create fallback colored textures
    QImage createColorTexture(const QColor& color);
    Fruit* createFruit();
    
    void initializeCamera();
    bool isFruitHit(const cv::Point& point, Fruit* fruit, QTime currentTime);
    void convertCameraPointToGameSpace(const cv::Point& cameraPoint, float& gameX, float& gameZ);
};

#endif // GAMEWIDGET_H
