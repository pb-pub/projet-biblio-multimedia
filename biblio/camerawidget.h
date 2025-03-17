#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/opencv.hpp>

namespace Ui {
class CameraWidget;
}

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();

private slots:
    void updateFrame();

private:

    Ui::CameraWidget *ui;
    QTimer *timer;
    cv::VideoCapture cap;
};

#endif // CAMERAWIDGET_H
