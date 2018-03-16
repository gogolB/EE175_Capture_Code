import ftd2xx
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import time
import cv2

# Need to check this before opening...
device = ftd2xx.open(2);

print(device.getStatus());
print(device.getDeviceInfo());
print("Waiting to send start char");



# Puts the device in Sync FIFO
print(device.setBitMode(0xFF,0x40));
# Performance Stuff
print(device.setLatencyTimer(2));
print(device.setUSBParameters(0x10000,0x10000));
# 0x0100 = FLOW_RTS_CTS
#print(device.setFlowControl(0x0100, 0x0, 0x0));

w = 640
h = 480
Matrix = [[0 for x in range(w)] for y in range(h)] 
a = np.array(Matrix, dtype=np.uint8);
cv2.namedWindow("img", cv2.WINDOW_NORMAL);
cv2.imshow("img", a);
input("Press anykey to start...");
print(device.write('%c' % 0xaa));

print("Entering read loop..");
device.setTimeouts(1000,0);
frame = 0;
fstart = time.time();
while(True):
	# Read data for one image.
	start = time.time(); 
	#Matrix = device.read(h*w*2);
	if(frame == 10):
		Matrix = list(map(int,device.read(h*w)));
		if(frame == 10):
			a = np.matrix(Matrix, dtype=np.uint8)
			a = np.reshape(a,(w,h));
			print(np.average(a));
			print(a.shape);
			cv2.imshow("img",a);
			cv2.waitKey(1);
	else:
		Matrix = device.read(h*w);
		#print("size of matrix %d",len(Matrix));
	end = time.time();
	if ((end - start) > 0 and len(Matrix) == w*h):
		#print("Done reading for one image, Speed: %f (MBps)" % (((w*h)/(end - start))/1e6))
		frame = frame +1;
		if(frame == 29):
			frame = 0;
			fend = time.time();
			print("FPS : %f" % ((30/(fend-fstart))))
			fstart = time.time();
	