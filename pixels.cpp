#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <ctime>

using namespace std;
using namespace cv;
using namespace gpu;

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

    //convert from BGR to HSV
    Mat img_hsv;
    cvtColor(img, img_hsv, COLOR_BGR2HSV);

    //split the hsv image into three channels
    vector<Mat> hsv_split;
    split(img_hsv, hsv_split);
    int x = img.cols;
    int y = img.rows;
    cout<<x<<endl;
    cout<<y<<endl;
    int i,j;
 
    clock_t start = clock();
    
    //threshold hue
    for ( j = 0; j < y; j++)
    {
        for (i = 0; i < x; i++)
        {
            int a = hsv_split[0].at<uchar>(j,i);
            if (a >= lowH && a <= highH)
            {
                hsv_split[0].at<uchar>(j,i) = 255;
            }
            else hsv_split[0].at<uchar>(j,i) = 0;
        }
    }

    //threshold saturation
    for ( j = 0; j < y; j++)
    {
        for (i = 0; i < x; i++)
        {
            int a = hsv_split[1].at<uchar>(j,i);
            if (a > lowS && a < highS)
            {
                hsv_split[1].at<uchar>(j,i) = 255;
            }
            else hsv_split[1].at<uchar>(j,i) = 0;
        }
    }

    //threshold value
    for ( j = 0; j < y; j++)
    {
        for (i = 0; i < x; i++)
        {
            int a = hsv_split[2].at<uchar>(j,i);
            if (a > lowV && a < highV)
            {
                hsv_split[2].at<uchar>(j,i) = 255;
            }
            else hsv_split[2].at<uchar>(j,i) = 0;
        }
    }

    //combine different planes
    Mat temp;
    Mat img_th;
    bitwise_and(hsv_split[0], hsv_split[1], temp);
    bitwise_and(hsv_split[2], temp, img_th);

    clock_t end = clock();
    double elapsed = double(end-start)/CLOCKS_PER_SEC;
    cout<<"Total time: "<<elapsed<<" sec"<<endl;

    //show processed image
    imshow("Here you go!", img_th);

    //press a button to quit the program
    waitKey(0);

    return 0;
}
