#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/objdetect.hpp>

#include <cstdio>
#include <iostream>

using namespace cv;
using namespace std;


int main(int, char**)
{
    int frameWidth=640;
    int frameHeight=480;

    VideoCapture cap("http://161.3.63.139:8000/camera/mjpeg", cv::CAP_FFMPEG); // open the default camera
    cout<<"width :"<<cap.get(CAP_PROP_FRAME_WIDTH)<<endl;
    cout<<"height :"<<cap.get(CAP_PROP_FRAME_HEIGHT)<<endl;
    cap.set(CAP_PROP_FRAME_WIDTH,frameWidth);
    cap.set(CAP_PROP_FRAME_HEIGHT,frameHeight);
    if(!cap.isOpened())  // check if we succeeded
    {
        cerr<<"Error openning the default camera"<<endl;
        return -1;
    }

    CascadeClassifier face_cascade;
    if( !face_cascade.load( "../../haarcascade_frontalface_alt.xml" ) )
    {
        cerr<<"Error loading haarcascade"<<endl;
        return -1;
    }

    // Init output window
    namedWindow("WebCam",1);

    while (waitKey(5)<0)
    {
        Mat frame,frame_gray;
        std::vector<Rect> faces;
        // Get frame
        cap >> frame;
        // Mirror effect
        cv::flip(frame,frame,1);
        // Convert to gray
        cv::cvtColor(frame,frame_gray,COLOR_BGR2GRAY);
        // Equalize graylevels
//        equalizeHist( frame_gray, frame_gray );
        //-- Detect faces
        face_cascade.detectMultiScale( frame_gray, faces, 1.1, 4, 0, Size(60, 60) );
        if (faces.size()>0)
        {
            // Draw green rectangle
            for (int i=0;i<(int)faces.size();i++)
                rectangle(frame,faces[i],Scalar(0,255,0),2);
        }

        // Display frame
        imshow("WebCam", frame);
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
