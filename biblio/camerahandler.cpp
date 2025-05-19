#include "camerahandler.h"
#include <QDebug>
#include <iostream>
#include <vector>

CameraHandler::CameraHandler()
{
    loadFaceCascade();
}

CameraHandler::~CameraHandler()
{
    if (cap.isOpened())
    {
        cap.release();
    }
}

bool CameraHandler::loadFaceCascade()
{
    QString cascadePath = "/Users/ismail/projet-biblio-multimedia/biblio/assets/fist.xml";
    if (!faceCascade.load(cascadePath.toStdString()))
    {
        cascadePath = "./../../../biblio/assets/fist.xml";
        if (!faceCascade.load(cascadePath.toStdString()))
        {
            qDebug() << "Error: Could not load Haar Cascade from" << cascadePath;
            return false;
        }
    }
    return true;
}

int CameraHandler::openCamera()
{
    // First try - explicitly use AVFOUNDATION backend for macOS
    cap.open(0, cv::CAP_AVFOUNDATION);
    if (cap.isOpened())
        return 1;

// Check if camera access was denied (specific to macOS)
#ifdef __APPLE__
    // Log information about the camera access attempt
    qDebug() << "Attempting to access camera with AVFoundation backend";
    cv::VideoCapture testCap(0, cv::CAP_AVFOUNDATION);
    if (!testCap.isOpened())
    {
        // On macOS, status 0 typically means permission denied
        qDebug() << "Camera access appears to be denied (permission issue)";
        return -1; // Return -1 for permission issues
    }
#endif

    // Second try - default camera with default backend
    cap.open(0);
    if (cap.isOpened())
        return 1;

    cap.open("http://192.168.1.80:8000/camera/mjpeg", cv::CAP_FFMPEG);
    if (cap.isOpened())
        return 1;

    cap.open("http://161.3.37.158:8000/camera/mjpeg", cv::CAP_FFMPEG);
    if (cap.isOpened())
        return 1;

    qDebug() << "Error: Could not open camera or video source";
    return 0; 
}

bool CameraHandler::isOpened() const
{
    return cap.isOpened();
}

bool CameraHandler::getFrame(cv::Mat &frame)
{
    if (!cap.isOpened())
    {
        return false;
    }

    cap >> frame;
    return !frame.empty();
}

std::vector<cv::Point> CameraHandler::detectFaces(cv::Mat &frame, cv::Mat &grayFrame, bool thresholdingEnabled)
{
    std::vector<cv::Point> detectedPoints;

    if (thresholdingEnabled)
    {
        cv::threshold(grayFrame, grayFrame, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    }

    if (!faceCascade.empty())
    {
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 6,
                                     0 | cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_SCALE_IMAGE,
                                     cv::Size(30, 30), cv::Size(300, 300));

        for (const auto &face : faces)
        {
            // Draw rectangles around detected faces
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

            // Add center point to detected points
            cv::Point center(face.x + face.width / 2, face.y + face.height / 2);
            detectedPoints.push_back(center);
            std::cout << "Face center: (" << center.x << ", " << center.y << ")" << std::endl;
            cv::circle(frame, center, 5, cv::Scalar(255, 0, 0), -1);
        }
    }

    return detectedPoints;
}
