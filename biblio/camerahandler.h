#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <QString>
#include <vector>

class CameraHandler
{
public:
    CameraHandler();
    ~CameraHandler();

    int openCamera(); // Returns status code: 1=success, 0=no camera, -1=permission denied
    bool isOpened() const;
    bool getFrame(cv::Mat& frame);
    
    // Update to return detected points
    std::vector<cv::Point> detectFaces(cv::Mat& frame, cv::Mat& grayFrame, bool thresholdingEnabled);

    
    
private:
    cv::VideoCapture cap;
    cv::CascadeClassifier faceCascade;
    bool loadFaceCascade();
};

#endif // CAMERAHANDLER_H
