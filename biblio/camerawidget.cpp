#include "camerawidget.h"
#include "ui_camerawidget.h"
#include <QDebug>

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CameraWidget), cap("http://192.168.1.80:8000/camera/mjpeg", cv::CAP_FFMPEG)
{
    ui->setupUi(this);

    if (!cap.isOpened()) {
        qDebug() << "Error: Could not open camera";
        return;
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CameraWidget::updateFrame);
    timer->start(33);
}

CameraWidget::~CameraWidget()
{
    delete timer;
    delete ui;
}

void CameraWidget::updateFrame()
{
    cv::Mat frame;
    cap >> frame;

    if (frame.empty())
        return;

    // Convert BGR (OpenCV default) to RGB (Qt expects RGB format)
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    // Get QLabel dimensions
    int labelWidth = ui->label->width();
    int labelHeight = ui->label->height();

    // Resize the frame to match QLabel size while maintaining aspect ratio
    cv::Mat resizedFrame;
    double aspectRatio = (double)frame.cols / (double)frame.rows;
    int newWidth = labelWidth;
    int newHeight = static_cast<int>(labelWidth / aspectRatio);

    if (newHeight > labelHeight) {
        newHeight = labelHeight;
        newWidth = static_cast<int>(labelHeight * aspectRatio);
    }

    cv::resize(frame, resizedFrame, cv::Size(newWidth, newHeight), cv::INTER_LINEAR);

    // Convert cv::Mat to QImage
    QImage qimg(resizedFrame.data, resizedFrame.cols, resizedFrame.rows, resizedFrame.step, QImage::Format_RGB888);

    // Display image on QLabel
    ui->label->setPixmap(QPixmap::fromImage(qimg));
}
