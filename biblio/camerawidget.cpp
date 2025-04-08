#include "camerawidget.h"
#include "ui_camerawidget.h"
#include <QDebug>
#include <opencv2/objdetect.hpp>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CameraWidget)
{
    ui->setupUi(this);

    // Set up the QLabel to display the full image
    ui->label->setMinimumSize(1280, 360);
    ui->label->setScaledContents(true);
    ui->label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    ui->label->setAlignment(Qt::AlignCenter);

    // Try different methods to open the camera
    int openStatus = openCamera();
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
            // No camera found or other error
            QMessageBox::warning(this, "Camera Error", 
                            "Failed to open camera. No camera device was found or it's already in use.");
        }
        
        showPlaceholderMessage("Camera access required", 
                              "Please grant camera permission in System Settings → Privacy & Security → Camera");
    }

    
    QString cascadePath = "/Users/ismail/projet-biblio-multimedia/biblio/assets/haarcascade_frontalface_alt.xml";
    if (!faceCascade.load(cascadePath.toStdString())) {
        cascadePath = "./../../../biblio/assets/haarcascade_frontalface_alt.xml";
        if (!faceCascade.load(cascadePath.toStdString())) {
            qDebug() << "Error: Could not load Haar Cascade from" << cascadePath;
        }
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CameraWidget::updateFrame);
    timer->start(33);

    std::cout << "CameraWidget initialized" << std::endl;
}

int CameraWidget::openCamera() {
    // First try - explicitly use AVFOUNDATION backend for macOS
    cap.open(0, cv::CAP_AVFOUNDATION);
    if (cap.isOpened()) return 1;
    
    // Check if camera access was denied (specific to macOS)
    #ifdef __APPLE__
    // Log information about the camera access attempt
    qDebug() << "Attempting to access camera with AVFoundation backend";
    cv::VideoCapture testCap(0, cv::CAP_AVFOUNDATION);
    if (!testCap.isOpened()) {
        // On macOS, status 0 typically means permission denied
        qDebug() << "Camera access appears to be denied (permission issue)";
        return -1; // Return -1 for permission issues
    }
    #endif
    
    // Second try - default camera with default backend
    cap.open(0);
    if (cap.isOpened()) return 1;
    
    // Third try - try different camera index
    cap.open("http://161.3.36.211:8000/camera/mjpeg", cv::CAP_FFMPEG);
    if (cap.isOpened()) return 1;
    
    
    qDebug() << "Error: Could not open camera or video source";
    return 0; // Return 0 for other errors
}

void CameraWidget::showPlaceholderMessage(const QString &title, const QString &message) {
    // Create a placeholder image with the message
    cv::Mat placeholder(480, 640, CV_8UC3, cv::Scalar(40, 40, 40));
    
    // Add the title text
    cv::putText(placeholder, title.toStdString(), cv::Point(50, 100), 
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    // Add the message text (possibly on multiple lines)
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
    
    // Convert to QImage and display
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
    
    // Check if camera is open
    if (!cap.isOpened()) {
        // Try to reopen the camera periodically
        static int reopenCounter = 0;
        if (++reopenCounter >= 90) { // Try every ~3 seconds (90 * 33ms)
            reopenCounter = 0;
            int status = openCamera();
            if (status > 0) {
                qDebug() << "Camera reconnected successfully";
            }
        }
        
        // If camera isn't open, exit early to keep showing the placeholder message
        return;
    }
    
    // Try to read a frame
    cap >> frame;
    if (frame.empty()) {
        showPlaceholderMessage("No Frame Received", "Camera is connected but no video stream is available");
        return;
    }

    // Convert BGR (OpenCV default) to RGB (Qt expects RGB format)
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    // Convert to grayscale for face detection
    cv::Mat frameGray = frame.clone();
    cv::cvtColor(frameGray, frameGray, cv::COLOR_RGB2GRAY);

    if (thresholdingEnabled) 
        cv::threshold(frameGray, frameGray, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    // Detect faces only if cascade was loaded
    if (!faceCascade.empty()) {
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(frameGray, faces, 1.1, 4, 0 | cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30), cv::Size(300, 300));
        
        // Draw rectangles around detected faces
        for (const auto &face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            
            // Print the center of the face rectangle
            std::cout << "Face center: (" << face.x + face.width / 2 << ", " << face.y + face.height / 2 << ")" << std::endl;

            // Draw a circle at the center of the face
            cv::circle(frame, cv::Point(face.x + face.width / 2, face.y + face.height / 2), 5, cv::Scalar(255, 0, 0), -1);
        }

    }

    // Convert the grayscale to color for display
    cv::Mat frame2;
    cv::cvtColor(frameGray, frame2, cv::COLOR_GRAY2RGB);
    
    // Create side-by-side display (concatenate horizontally)
    cv::Mat combinedFrame;
    cv::hconcat(frame, frame2, combinedFrame);

    // Convert cv::Mat to QImage
    QImage qimg(combinedFrame.data, combinedFrame.cols, combinedFrame.rows, combinedFrame.step, QImage::Format_RGB888);

    // Get the size of the label
    QSize labelSize = ui->label->size();
    
    // Create a pixmap from the image
    QPixmap pixmap = QPixmap::fromImage(qimg);
    
    // Scale to fill the entire label while maintaining aspect ratio
    QPixmap scaledPixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Set the scaled pixmap to the label
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

