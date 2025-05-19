#include "camerawidget.h"
#include "ui_camerawidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CameraWidget)
{
    ui->setupUi(this);

    ui->label->setMinimumSize(1280, 360);
    ui->label->setScaledContents(true);
    ui->label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    ui->label->setAlignment(Qt::AlignCenter);

    int openStatus = cameraHandler.openCamera();
    if (openStatus <= 0) {
        // Show different messages based on the error
        if (openStatus == -1) {
            // Permission denied
            QMessageBox::warning(this, "Camera Permission Required", 
                "This application needs access to your camera.\n\n"
                "Please go to System Settings → Privacy & Security → Camera\n"
                "and enable permission for this application.",
                QMessageBox::Ok);
            
            // Offer to open system settings
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Open Settings", 
                                        "Would you like to open System Settings now?",
                                        QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                // Open macOS Privacy settings for Camera
                QDesktopServices::openUrl(QUrl("x-apple.systempreferences:com.apple.preference.security?Privacy_Camera"));
            }
        } else {
            QMessageBox::warning(this, "Camera Error", 
                            "Failed to open camera. No camera device was found or it's already in use.");
        }
        
        showPlaceholderMessage("Camera access required", 
                              "Please grant camera permission in System Settings → Privacy & Security → Camera");
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CameraWidget::updateFrame);
    timer->start(33);

    std::cout << "CameraWidget initialized" << std::endl;
}

void CameraWidget::showPlaceholderMessage(const QString &title, const QString &message)
{
    cv::Mat placeholder(480, 640, CV_8UC3, cv::Scalar(40, 40, 40));
    
    cv::putText(placeholder, title.toStdString(), cv::Point(50, 100), 
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    QStringList lines = message.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        cv::putText(placeholder, lines[i].toStdString(), cv::Point(50, 150 + i * 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(200, 200, 200), 1);
    }
    
    // Draw a camera icon or symbol
    int centerX = placeholder.cols / 2;
    int centerY = placeholder.rows / 2 + 50;
    int radius = 40;
    cv::circle(placeholder, cv::Point(centerX, centerY), radius, cv::Scalar(100, 100, 255), 2);
    cv::circle(placeholder, cv::Point(centerX, centerY), radius/2, cv::Scalar(100, 100, 255), -1);
    cv::line(placeholder, cv::Point(centerX + radius, centerY - radius), 
            cv::Point(centerX + radius + 20, centerY - radius - 20), 
            cv::Scalar(100, 100, 255), 2);
    
    cv::cvtColor(placeholder, placeholder, cv::COLOR_BGR2RGB);
    QImage img(placeholder.data, placeholder.cols, placeholder.rows, placeholder.step, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(img));
}

CameraWidget::~CameraWidget()
{
    delete timer;
    delete ui;
}

void CameraWidget::updateFrame()
{
    cv::Mat frame;
    
    if (!cameraHandler.isOpened()) {
        static int reopenCounter = 0;
        if (++reopenCounter >= 90) { // Try every ~3 seconds (90 * 33ms)
            reopenCounter = 0;
            int status = cameraHandler.openCamera();
            if (status > 0) {
                qDebug() << "Camera reconnected successfully";
            }
        }
        
        // If camera isn't open, exit early to keep showing the placeholder message
        return;
    }
    
    if (!cameraHandler.getFrame(frame)) {
        showPlaceholderMessage("No Frame Received", "Camera is connected but no video stream is available");
        return;
    }

    // Convert BGR (OpenCV default) to RGB (Qt expects RGB format)
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    cv::Mat frameGray = frame.clone();
    cv::cvtColor(frameGray, frameGray, cv::COLOR_RGB2GRAY);

    cameraHandler.detectFaces(frame, frameGray, thresholdingEnabled);

    cv::Mat frame2;
    cv::cvtColor(frameGray, frameGray, cv::COLOR_GRAY2RGB);
    
    cv::Mat combinedFrame;
    cv::hconcat(frame, frameGray, combinedFrame);

    // Convert cv::Mat to QImage
    QImage qimg(combinedFrame.data, combinedFrame.cols, combinedFrame.rows, combinedFrame.step, QImage::Format_RGB888);

    QSize labelSize = ui->label->size();
    
    QPixmap pixmap = QPixmap::fromImage(qimg);
    
    QPixmap scaledPixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    ui->label->setPixmap(scaledPixmap);
}

void CameraWidget::on_thresholdingButton_clicked()
{
    thresholdingEnabled = !thresholdingEnabled;
    if (thresholdingEnabled)
        ui->thresholdingButton->setText("Disable Thresholding");
    else
        ui->thresholdingButton->setText("Enable Thresholding");
}

