#include "camerawidget.h"
#include "ui_camerawidget.h"
#include <QDebug>
#include <opencv2/objdetect.hpp>

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::CameraWidget), cap("http://161.3.45.205:8000/camera/mjpeg", cv::CAP_FFMPEG)
{
    ui->setupUi(this);

    // Set up the QLabel to display the full image
    ui->label->setMinimumSize(1280, 360);
    ui->label->setScaledContents(true);
    ui->label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    ui->label->setAlignment(Qt::AlignCenter);

    if (!cap.isOpened()) {
        qDebug() << "Error: Could not open camera";
        return;
    }

    // Load the Haar Cascade for face detection
    if (!faceCascade.load("/home/paulb/Documents/Cours/projet-BM/projet-biblio-multimedia/biblio/haarcascade_frontalface_alt.xml")) {
        qDebug() << "Error: Could not load Haar Cascade";
        return;
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CameraWidget::updateFrame);
    timer->start(33);

    std::cout<< "CameraWidget initialized" << std::endl;
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

    // Convert to grayscale for face detection
    cv::Mat frameGray = frame.clone();
    cv::cvtColor(frameGray, frameGray, cv::COLOR_RGB2GRAY);
    cv::threshold(frameGray, frameGray, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Detect faces
    std::vector<cv::Rect> faces;
    faceCascade.detectMultiScale(frameGray, faces, 1.1, 4, 0 | cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30), cv::Size(300, 300));
    
    // Draw rectangles around detected faces
    for (const auto &face : faces) {
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    }

    // Convert the grayscale to color for display
    cv::Mat frame2;
    cv::cvtColor(frameGray, frame2, cv::COLOR_GRAY2RGB);
    
    // Create side-by-side display (concatenate horizontally)
    cv::Mat combinedFrame;
    cv::hconcat(frame, frame2, combinedFrame);

    // Convert cv::Mat to QImage
    QImage qimg(combinedFrame.data, combinedFrame.cols, combinedFrame.rows, combinedFrame.step, QImage::Format_RGB888);

    std::cout << "Frame size: " << combinedFrame.cols << "x" << combinedFrame.rows << std::endl;
    std::cout << "Label size: " << ui->label->width() << "x" << ui->label->height() << std::endl;
    
    // Get the size of the label
    QSize labelSize = ui->label->size();
    
    // Create a pixmap from the image
    QPixmap pixmap = QPixmap::fromImage(qimg);
    
    // Scale to fill the entire label while maintaining aspect ratio
    // Use the entire width of the label
    int scaledWidth = labelSize.width();
    // Calculate height while maintaining aspect ratio
    int scaledHeight = (int)(scaledWidth * ((double)pixmap.height() / pixmap.width()));
    
    // If the scaled height is still less than the label height, scale based on height instead
    if (scaledHeight < labelSize.height()) {
        scaledHeight = labelSize.height();
        scaledWidth = (int)(scaledHeight * ((double)pixmap.width() / pixmap.height()));
    }
    
    // Scale the pixmap to the calculated dimensions
    QPixmap scaledPixmap = pixmap.scaled(scaledWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Set the scaled pixmap to the label
    ui->label->setPixmap(scaledPixmap);
}
