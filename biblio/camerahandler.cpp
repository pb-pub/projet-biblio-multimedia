#include "camerahandler.h"
#include <QDebug>
#include <iostream>
#include <vector>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

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
    QStringList pathsToTry;
    QString appDir = QCoreApplication::applicationDirPath();

    // Path for macOS bundle (assets inside Resources folder)
    pathsToTry << appDir + "/../Resources/assets/fist.xml";
    // Path if assets folder is next to the executable
    pathsToTry << appDir + "/assets/fist.xml";
    
    // Paths relative to current working directory (useful for development/build dir execution)
    pathsToTry << "assets/fist.xml";
    pathsToTry << "../assets/fist.xml"; // If executable is in a subfolder like 'bin'
    pathsToTry << "../../assets/fist.xml";
    pathsToTry << "biblio/assets/fist.xml"; // If running from project root and executable is in biblio/
    pathsToTry << "../biblio/assets/fist.xml";
    pathsToTry << "../../biblio/assets/fist.xml";
    pathsToTry << "./../../../biblio/assets/fist.xml";

    // Path for when fist.xml is in the same directory as the executable (less likely for 'assets' structure)
    pathsToTry << appDir + "/fist.xml";
    pathsToTry << "fist.xml";


    for (const QString& cascadePath : pathsToTry) {
        QFileInfo fileInfo(cascadePath);
        qDebug() << "Attempting to load Haar Cascade from:" << QDir::cleanPath(fileInfo.absoluteFilePath());
        if (faceCascade.load(QDir::cleanPath(fileInfo.absoluteFilePath()).toStdString())) {
            qDebug() << "Successfully loaded Haar Cascade from:" << QDir::cleanPath(fileInfo.absoluteFilePath());
            return true;
        }
    }
    
    // Attempt to locate via Qt Resource system if it were used (example, not active here)
    // pathsToTry << ":/assets/fist.xml";

    // Final attempt with a path that might be set via an environment variable or a known install location
    // This part would be specific to deployment strategy if assets are installed system-wide
    // For example, using QStandardPaths:
    QString dataLocation = QStandardPaths::locate(QStandardPaths::AppDataLocation, "assets/fist.xml");
    if (!dataLocation.isEmpty()) {
         qDebug() << "Attempting to load Haar Cascade from AppDataLocation:" << dataLocation;
        if (faceCascade.load(dataLocation.toStdString())) {
            qDebug() << "Successfully loaded Haar Cascade from AppDataLocation:" << dataLocation;
            return true;
        }
    }


    qDebug() << "Error: Could not load Haar Cascade ('fist.xml'). All attempted paths failed.";
    qDebug() << "Ensure 'fist.xml' is present in one of the expected 'assets' directory locations relative to the executable or build directory.";
    return false;
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
