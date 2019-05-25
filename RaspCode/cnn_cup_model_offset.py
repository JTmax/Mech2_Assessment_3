import cv2 as cv
import glob
import os
import numpy
import serial
from picamera.array import PiRGBArray
from picamera import PiCamera
import time

font = cv.FONT_HERSHEY_DUPLEX

CONF = 0.6

ser = serial.Serial(

  port='/dev/ttyS0',
  baudrate = 9600,
  stopbits=serial.STOPBITS_ONE,
  bytesize=serial.EIGHTBITS,
  timeout=0.1,
  write_timeout=0.1)

# initialize the camera and grab a reference to the raw camera capture
camera = PiCamera()
rawCapture = PiRGBArray(camera)

# Wait for the automatic gain control to settle
time.sleep(2)

# Now fix the values
camera.shutter_speed = camera.exposure_speed
camera.exposure_mode = 'off'
g = camera.awb_gains
camera.awb_mode = 'off'
camera.awb_gains = g

cvNet = cv.dnn.readNetFromTensorflow('sorted_inference_graph.pb', 'graph.pbtxt')

count = 0

while True:

    serData = ser.readline()
    #print(serData)
    if serData == b'1\r\n':
        print("Testing image")
        #Take image
        rawCapture = PiRGBArray(camera)
        camera.capture(rawCapture, format="bgr")
                        
        img = rawCapture.array
        img = cv.flip(img, -1)

        height, width, channels = img.shape
        
        scr_x = round(width/2)
        scr_y = round(height/2)

        cvNet.setInput(cv.dnn.blobFromImage(img, size=(300, 300), swapRB=True, crop=False))
        cvOut = cvNet.forward()

        
        netMax = 0
        arrayMax = [0,0,0,0,0,0,0]
        
        for detection in cvOut[0,0,:,:]:
            
            if float(detection[2]) > netMax:
                netMax = detection[2]
                arrayMax = detection
        
		# print(arrayMax)
        
		# print(cvOut)
            
        score = float(arrayMax[2])

        if score > CONF:

            left = int(arrayMax[3] * width)
            top = int(arrayMax[4] * height)
            right = int(arrayMax[5] * width)
            bottom = int(arrayMax[6] * height)

            cx = round(left - (left- right)/2)
            cy = round(top - (top- bottom)/2)

            cv.line(img,(cx,scr_y),(scr_x,scr_y),(255,0,0),2)
            cv.line(img,(cx,scr_y),(cx,cy),(255,0,0),2)

            #Cross hairs
            cv.line(img,(scr_x-20,scr_y),(scr_x+20,scr_y),(238,244,21),3)
            cv.line(img,(scr_x,scr_y-20),(scr_x,scr_y+20),(238,244,21),3)

            cv.circle(img,(cx,cy), 3, (0,0,255), -1)
            
            Offest = scr_x - cx

            print("Offest X: {}".format(Offest))

            #Detection box
            cv.rectangle(img, (left, top), (right, bottom), (23, 230, 210), thickness=2)

            #Put text on image
            cv.putText(img,"X_Of:"+str(Offest)+"px",(10,430), font, 2,(0,0,0),3,cv.LINE_AA)
            cv.putText(img,"P():"+str(round(score*100))+"%",(10,500), font, 2,(0,0,0),3,cv.LINE_AA)

            #Save image
            cv.imwrite('detect_img/{}.png'.format(count),img)
            cv.imwrite('/home/pi/Desktop/Website/img/img.png',img)
            count += 1
            
            print("Found")

            serilString = "1,"+ str(Offest)+"|"
            ser.write(serilString.encode('utf-8'))
        
        else:
            print("Not found")
            serilString = "0,0000|"
            ser.write(serilString.encode('utf-8'))
            cv.imwrite('/home/pi/Desktop/Website/img/img.png',img)


    else:
        ser.flush()
    