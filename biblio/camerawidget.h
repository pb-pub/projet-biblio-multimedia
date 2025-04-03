#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

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
    cv::VideoCapture cap;
    QTimer *timer;
    cv::CascadeClassifier faceCascade; // Added for face detection
};

#endif // CAMERAWIDGET_H
