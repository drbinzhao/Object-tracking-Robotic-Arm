#include <iostream>
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

int main(){
    //read the image
    Mat img = imread("red_ball.jpg", CV_LOAD_IMAGE_UNCHANGED);
    
    if (img.empty()){
        cout << "Failed to load image\n";
        return -1;
    }

    //open a window
    namedWindow("Capture the red ball", CV_WINDOW_AUTOSIZE);
    //show the image in the window
    imshow("Capture the red ball", img);

    //suspend and wait for a key press so that user can see the image 
    waitKey(0);

    destroyWindow("Capture the red ball");

    return 0;
}
