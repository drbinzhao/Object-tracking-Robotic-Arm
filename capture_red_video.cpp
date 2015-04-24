#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

int main(){
    
    int lowH, highH, lowS, highS, lowV, highV;
    int lastX, lastY;

    //open the camera
    VideoCapture cam(0);

    //check if opened correctly
    if (!cam.isOpened()){
        cout<<"Error: Failed to open camera"<<endl;
        return -1;
    }

    //capture a frame to get camera output size and total pixels for later use
    Mat test;
    cam.read(test);

    //get total pixel counts
    int nPixels = test.total();
    cout<<"Total pixel count is: "<<nPixels<<endl;

    //initialize as a black image for later use when drawing lines
    Mat img_line = Mat::zeros(test.size(), CV_8UC3);

    //initialize lastX and lastY
    lastX = -1;
    lastY = -1;

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

        //convert from BGR to HSV
        Mat img_hsv;
        cvtColor(img, img_hsv, COLOR_BGR2HSV);
/*  
        //display the hsv image 
        namedWindow("HSV", CV_WINDOW_AUTOSIZE);
        imshow("HSV", img_hsv);
*/        
        //threshold the image
        Mat img_th;
        inRange(img_hsv, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), img_th);
        
        //Morphological operations
        //opening
        //remove small objects(black) from the foreground
        erode(img_th, img_th, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        dilate(img_th, img_th, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));

        //closing
        //remove small holes(white) from the foreground
        dilate(img_th, img_th, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        erode(img_th, img_th, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));

        //to get the center of the object, we calculate the moment
        //a moment is a specific quantitive measure
        //the 0th moment is the total value, and the 1st moment divided 
        //by the total value (0th moment) is the center of the value
        //the 2nd moment is the rotational inertia

        //calculate moments of the thresholded image
        Moments moment = moments(img_th);

        double M_x = moment.m01;
        double M_y = moment.m10;
        double total_area = moment.m00;
    
        //only track when object size is at least 1/1000 of the frame size
        //reduce possible noise interference
        if (total_area > nPixels/1000){

        int newX = M_x/total_area;
        int newY = M_y/total_area;

        //draw a line if object found within range
        if (lastX >= 0 && lastY >= 0 && newX >= 0 && newY >= 0){
            line(img_line, Point(newX, newY), Point(lastX, lastY), Scalar(0,0,255),4);
        }

        //update current position
        lastX = newX;
        lastY = newY;
        }

        //display the thresholded image
        namedWindow("THRESHOLDED", CV_WINDOW_AUTOSIZE);
        imshow("THRESHOLDED", img_th);
    
        //display the original image
        img = img + img_line;
        namedWindow("Original", CV_WINDOW_AUTOSIZE);
        imshow("Original", img);

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

