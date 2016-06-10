#! /usr/bin/python

import time
import serial

file = open("alm.bin","rb")
alm = file.read()
print(alm)
file.close()

ser = serial.Serial(
    port="/dev/tty.usbmodem1412",
    baudrate=9600,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_ONE,
)

for c in alm:
	#print(c)
	ser.write(c)
	print ser.read()

ser.close()


