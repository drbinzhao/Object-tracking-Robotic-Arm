
/*This program detects and tracks a red object on Nvidia's Jetson embedded board (Keplar GPU w/ 192 cores) with OpenCV*/
/*Achieved 4x times total speed up with current implementation*/ 

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <ctime>
using namespace cv;
using namespace std;
using namespace gpu;

//libraries for the socket and some C functions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG_SIZE 40

void error(const char *);

int sockfd, n;
unsigned len;
struct hostent *hp;
char buffer[MSG_SIZE];

//create client, server structure
struct sockaddr_in serv_addr, cli_addr;
socklen_t ser_len, cli_len;

//function to send coordinates to the socket
int send_socket(char* buffer){
    
    //send message to server
    n = sendto(sockfd, buffer, MSG_SIZE, 0, (const struct sockaddr *)&serv_addr, len);
    if (n < 0){ 
        cout<<"Error sending the coordinates."<<endl;
        return -1;
    }
}

int main(int argc, char *argv[]){
    
    int lowH, highH, lowS, highS, lowV, highV;
    int lastX, lastY;

    /*--------------socket---------------*/

    //check if user specify the server's ip address and a port number
    if (argc != 3){
        cout<<"Please specify the host name and port for "<<argv[0]<<endl;
        return -1;
    }

    //open a connectionless UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        cout<<"Error on opening socket."<<endl;
        return -1;
    }

    //set internet domain
    serv_addr.sin_family = AF_INET;
    //converts hostname to ip address
    hp = gethostbyname(argv[1]);
    if (hp == 0){ 
        cout<<"Unknown host"<<endl;
        return -1;
    }

    //copy host ip into server ip address
    bcopy((char *)hp->h_addr, (char *)&serv_addr.sin_addr, hp->h_length);
    //set the port
    serv_addr.sin_port = htons(atoi(argv[2]));
    len = sizeof(struct sockaddr_in);

    /*-------------object capturing-------------*/

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

    //setting GPU
    setDevice(0);

    //get the live video frame by frame
    while(1){
 
        //source image from host
        Mat h_src;
        
        //check if captured correctly while reading a new frame from video
        if (!cam.read(h_src)){
            cout<<"Error: Failed to read image"<<endl;
            break;
        }

        
        clock_t begin = clock();

        //transfer host image to device
        GpuMat d_src;
        d_src.upload(h_src);
        GpuMat d_hsv;

        //convert from BGR to HSV on GPU
        cvtColor(d_src, d_hsv, COLOR_BGR2HSV);

        //threshold the image
        //split hsv image into three channels to use one channel threshold function on GPU
        //GPU doesn't support inRange() function
        vector<GpuMat> hsv_split;
        split(d_hsv, hsv_split);

        GpuMat d_h, d_s, d_v, temp, d_th;

        //conduct thresholding for each plane on GPU
        threshold(hsv_split[0], d_h, lowH, highH, THRESH_BINARY);
        threshold(hsv_split[1], d_s, lowS, highS, THRESH_BINARY);
        threshold(hsv_split[2], d_v, lowV, highV, THRESH_BINARY);
    
        //add each plane back together
        bitwise_and(d_h, d_s, temp);
        bitwise_and(d_v, temp, d_th);

        //conduct morphological operations on the GPU
        GpuMat d_morph = d_th;
        GpuMat p1, p2, p3;

        //Morphological operations, GPU doesn't support in-place morph operations
        //opening
        //remove small objects(black) from the foreground
        erode(d_morph, p1, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        dilate(p1, p2, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));

        //closing
        //remove small holes(white) from the foreground
        dilate(p2, p3, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        erode(p3, d_morph, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        
        //copy final result back to host
        Mat h_morph(d_morph);  
        
        clock_t end = clock();    
        double elapsed = double(end-begin)/CLOCKS_PER_SEC;
        cout<<"Total time (convert + threshold + morph): "<<elapsed<<" secs"<<endl;

        //to get the center of the object, we calculate the moment
        //a moment is a specific quantitive measure
        //the 0th moment is the total value, and the 1st moment divided 
        //by the total value (0th moment) is the center of the value
        //the 2nd moment is the rotational inertia

        //calculate moments of the thresholded image
        Moments moment = moments(h_morph);

        double M_x = moment.m10;
        double M_y = moment.m01;
        double total_area = moment.m00;
    
        //only track when object size is at least 1/1000 of the frame size
        //reduce possible noise interference
        if (total_area > nPixels/1000){

        int newX = M_x/total_area;
        int newY = M_y/total_area;
        
        //clear the buffer
        bzero(buffer, MSG_SIZE);

        //send target to the socket
        sprintf(buffer, "x: %d y: %d", newX, newY);
        send_socket(buffer);
        
        //draw a line if object found within range
        if (lastX >= 0 && lastY >= 0 && newX >= 0 && newY >= 0){
            line(img_line, Point(newX, newY), Point(lastX, lastY), Scalar(0,0,255),4);
        }

        //update current position
        lastX = newX;
        lastY = newY;
        }

        //display the thresholded image
        namedWindow("Thresholded", CV_WINDOW_AUTOSIZE);
        imshow("Thresholded", h_morph);
    
        //display the original image
        h_src = h_src + img_line;
        namedWindow("Original", CV_WINDOW_AUTOSIZE);
        imshow("Original", h_src);

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

