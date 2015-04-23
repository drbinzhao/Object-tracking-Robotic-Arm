#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/gpu/gpu.hpp"

using namespace cv;
using namespace std;

int main(){
    //open the camera
    VideoCapture cam(0);

    //check if opened correctly
    if (!cam.isOpened()){
        cout<<"Error: Failed to open camera"<<endl;
        return -1;
    }

    //get the live video frame by frame
    while(1){
        
        Mat src;
        
        //check if captured correctly while reading a new frame from video
        if (!cam.read(src)){
            cout<<"Error: Failed to read image"<<endl;
            return -1;
        }

        //using GPU for image processing
        gpu::GpuMat d_src(src);
        gpu::GpuMat d_dst;

        //copy result back to host
        Mat dst(d_dst);
        
        //display the original image and the gpu image
        namedWindow("CPU", CV_WINDOW_AUTOSIZE);
        imshow("CPU", src);
//        namedWindow("GPU", CV_WINDOW_AUTOSIZE);
//        imshow("GPU", dst);

        //wait for space key to exit
        if ((waitKey(1) & 0xFF) == ' '){
            break;
        }
    }
    
    cam.release();
    destroyAllWindows();
    return 0;
}

