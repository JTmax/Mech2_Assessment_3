from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import imutils
import serial
import numpy

def loop():
	camera = PiCamera()
	rawCapture = PiRGBArray(camera)

	camera.resolution = (320, 240)
	camera.vflip = True 
	camera.framerate = 32

	# Wait for the automatic gain control to settle
	time.sleep(2)

	# Now fix the values
	camera.shutter_speed = camera.exposure_speed
	camera.exposure_mode = 'off'
	g = camera.awb_gains
	camera.awb_mode = 'off'
	camera.awb_gains = g

	camera.capture(rawCapture, format="bgr")

	cvNet = cv.dnn.readNetFromTensorflow('sorted_inference_graph.pb', 'graph.pbtxt')

	#setup seral link

	ser = serial.Serial(
	  port='/dev/ttyACM0',
	  baudrate = 115200,
	  stopbits=serial.STOPBITS_ONE,
	  bytesize=serial.EIGHTBITS,
	  timeout=1,
	  write_timeout=0 )

	while True:
		
		if readSerial(ser) == 1:
			image = rawCapture.array
			dtect, offest = model(image)
			sendMsg(dtect, offest)



def model(image):

	height, width, channels = img.shape

	# print(width)

	rows = img.shape[0]
	cols = img.shape[1]
	cvNet.setInput(cv.dnn.blobFromImage(img, size=(300, 300), swapRB=True, crop=False))
	cvOut = cvNet.forward()

	max = numpy.amax(cvOut[0,0,:,:])

	for detection in cvOut[0,0,:,:]:

		if detection[6] * rows > 270:
		
		    score = float(detection[2])

		    if score > 0.6:
		        left = detection[3] * cols
		        top = detection[4] * rows
		        right = detection[5] * cols
		        bottom = detection[6] * rows

		        cx = left - (left- right)/2
		        cy = top - (top- bottom)/2

		        cv.line(img,(int(cx),int(height/2)),(int(width/2),int(height/2)),(255,0,0),5)
		        cv.line(img,(int(width/2),int(height/2)),(int(width/2),int(cy)),(255,0,0),5)

		        cv.circle(img,(int(cx),int(cy)), 5, (0,0,255), -1)
		        
		        print("Offest X: {}".format(int(width/2 - cx)))

		        cv.rectangle(img, (int(left), int(top)), (int(right), int(bottom)), (23, 230, 210), thickness=2)

	
	return [dtect, offset]

def readSerial(ser):
	msg = ser.read_until()
	
	return (str)msg

def sendMsg(dtect, offest):
	msg = (str)dtect + "," str(offest)

if __name__ == "__main__":
	loop()