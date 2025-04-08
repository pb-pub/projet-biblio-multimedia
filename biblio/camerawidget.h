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
    void on_thresholdingButton_clicked();

private:
    Ui::CameraWidget *ui;
    cv::VideoCapture cap;
    QTimer *timer;
    cv::CascadeClassifier faceCascade; // Added for face detection
    
    int openCamera(); // Returns status code: 1=success, 0=no camera, -1=permission denied
    void showPlaceholderMessage(const QString &title, const QString &message);
    bool thresholdingEnabled = false; // Flag to enable/disable thresholding
    
};

#endif // CAMERAWIDGET_H
