import numpy as np
import cv2
import sys

#open the camera
cam = cv2.VideoCapture(0)

#check if camera is correctly opened
if (not cam.isOpened()):
    print("Error on opening webcam")
    sys.exit(-1)

#get the live video frame by frame until user hits space
while(True):
    ret, img = cam.read()
    cv2.imshow('Test', img)
    #set waitKey to enable frame refresh and 1ms for smooth live streaming 
    if cv2.waitKey(1) & 0xFF == ord(' '): 
        break

#release camera
cam.release()
cv2.destroyAllWindows()
