#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

int main(){
    
    int lowH, highH, lowS, highS, lowV, highV;
    
    //read img
    Mat img = imread("red_ball.jpg", CV_LOAD_IMAGE_UNCHANGED);

    //check if empty
    if (img.empty()){
        cout << "Failed to load img" << endl;
        return -1;
    }

    //show original image 
    imshow("Original", img);

    //set hue(0-179), saturati:on(0-255), and value(0-255) for red
    //red usually has h:170-179, s: 160-255, v: 60-255
    lowH = 170;
    highH = 179;
    lowS = 160;
    highS = 255;
    lowV = 60;
    highV = 255;

    //create control window
    namedWindow("Track Bars", CV_WINDOW_AUTOSIZE);

    //track bars
    cvCreateTrackbar("LowH", "Track Bars", &lowH, 179);
    cvCreateTrackbar("HighH", "Track Bars", &highH, 179);

    cvCreateTrackbar("LowS", "Track Bars", &lowS, 255);
    cvCreateTrackbar("HighS", "Track Bars", &lowS, 255);

    cvCreateTrackbar("LowV", "Track Bars", &lowV, 255);
    cvCreateTrackbar("HighV", "Track Bars", &highV, 255);

    //convert from BGR to HSV
    Mat img_hsv;
    cvtColor(img, img_hsv, COLOR_BGR2HSV);

    //threshold the image
    Mat img_th;
    inRange(img_hsv, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), img_th);

    //morph

    //show processed image
    imshow("Here you go!", img_th);

    //press a button to quit the program
    waitKey(0);

    return 0;
}
