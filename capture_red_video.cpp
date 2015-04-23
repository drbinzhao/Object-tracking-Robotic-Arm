#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

int main(){
    
    int lowH, highH, lowS, highS, lowV, highV;
   
    //open the camera
    VideoCapture cam(0);

    //check if opened correctly
    if (!cam.isOpened()){
        cout<<"Error: Failed to open camera"<<endl;
        return -1;
    }

    //create control window
    namedWindow("Track Bars", CV_WINDOW_AUTOSIZE);
    
    //set hue(0-179), saturati:on(0-255), and value(0-255) for red
    //red usually has h:170-179, s: 160-255, v: 60-255
    lowH = 170;
    highH = 179;
    lowS = 160;
    highS = 255;
    lowV = 60;
    highV = 255;

    //track bars
    cvCreateTrackbar("LowH", "Track Bars", &lowH, 179);
    cvCreateTrackbar("HighH", "Track Bars", &highH, 179);

    cvCreateTrackbar("LowS", "Track Bars", &lowS, 255);
    cvCreateTrackbar("HighS", "Track Bars", &lowS, 255);

    cvCreateTrackbar("LowV", "Track Bars", &lowV, 255);
    cvCreateTrackbar("HighV", "Track Bars", &highV, 255);

    //get the live video frame by frame
    while(1){
        
        Mat img;
        
        //check if captured correctly while reading a new frame from video
        if (!cam.read(img)){
            cout<<"Error: Failed to read image"<<endl;
            break;
        }

        //display the original image
        namedWindow("Original", CV_WINDOW_AUTOSIZE);
        imshow("Original", img);

        //convert from BGR to HSV
        Mat img_hsv;
        cvtColor(img, img_hsv, COLOR_BGR2HSV);
    
        //display the hsv image 
        namedWindow("HSV", CV_WINDOW_AUTOSIZE);
        imshow("HSV", img_hsv);
        
        //threshold the image
        Mat img_th;
        inRange(img_hsv, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), img_th);

        //display the thresholded image
        namedWindow("THRESHOLDED", CV_WINDOW_AUTOSIZE);
        imshow("THRESHOLDED", img_th);
    
        //enter space key to exit
        if ((waitKey(1) & 0xFF) == ' '){
            break;
        }
    }
    
    cam.release();
    destroyAllWindows();
    waitKey(0);
    return 0;
}

