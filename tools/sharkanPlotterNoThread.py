#!/usr/bin/env python

from serial import Serial
from threading import Thread
from time import sleep
import Queue
import time
import sys
import select
import math

NONE 	 = 0x00
REVERSED = 0x01
TWOCOMPL = 0x02

NPTSMAX  = 100
NCOLMAX	 = 3

def currMs():
	return int(round(time.time() * 1000))

def measureTime(f,func):
	t0 = currMs()
	f()
	dt = currMs() - t0
	print("%s took %dms" %(func,dt)) 

class EnhancedSerial(Serial):
	def __init__(self, *args, **kwargs):
		#ensure that a reasonable timeout is set
		timeout = kwargs.get('timeout',0.1)
		if timeout < 0.01: timeout = 0.1
		kwargs['timeout'] = timeout
		Serial.__init__(self, *args, **kwargs)
		self.buf = ''

	def readline(self, maxsize=None, timeout=1):
		"""maxsize is ignored, timeout in seconds is the max time that is way for a complete line"""
		tries = 0
		while 1:
			self.buf += self.read(512)
			pos = self.buf.find('\n')
			if pos >= 0:
				line, self.buf = self.buf[:pos+1], self.buf[pos+1:]
				return line
			tries += 1
			if tries * self.timeout > timeout:
				break
		line, self.buf = '', ''
		return line

	def readlines(self, sizehint=None, timeout=1):
		"""read all lines that are available. abort after timout
		when no more data arrives."""
		lines = []
		while 1:
			line = self.readline(timeout=timeout)
			if line:
				lines.append(line)
			if not line or line[-1:] != '\n':
				break
		return lines

class SerialReader(Thread):
	def __init__(self,queue,path,speed = None ):
		Thread.__init__(self)
		self.running = True
		#print("sr: init")
		self.q = queue
		if (path[0:8] == "/dev/tty") and (speed != None):
			print("Data Source is serial port %s @%d" %(path,speed))
			self.dataSource = EnhancedSerial(path, speed)
		else:
			print("Data Source is File %s" %path)
			self.dataSource = open(path,'r')

	def stop(self):
		self.running = False

	def run(self):
		while(self.running):
			#print("READER: RUN")
			count = 0
			v = self.dataSource.readline()
			self.q.put(v) 
			sleep(0.001)
		

class CanParser(Thread):
	def __init__(self, queue, data):
		Thread.__init__(self)
		self.q = queue
		self.data = data
		self.running = True

	def stop(self):
		self.running = False

	def canDbgParse(self,line):

		"""
		REDUCED example
		  31653870 2|3cd|8|FF FB 00 00 00 22 60 00
		  31663870 2|34d|7|00 03 FA FA 00 04 00
		  31673870 2|468|3|02 FF 00
		"""

		addr = 0 # addr <- [13:16]
		dataLen = 0 # dataLen <-  [17:18]
		data = 0 # data <- [19:]
		results = []

		try:
			addr = line[13:16]
			dataLen = line[17:18]
			data = line[19:].strip().split(" ")
			results = [addr,dataLen,data]
		except IndexError as e:
			print("Got Error type:(" + str(e) + ')')

		return results

	def convertData(self,bytes,converter = 0x00):
		
		l = len(bytes)
		s = ""

		# REVERSING BYTES
		if converter & REVERSED:
			bytes = list(reversed(bytes))

		# ASSEMBLE 
		for byte in bytes:
			s += byte

		# GO TO INT
		if converter & TWOCOMPL:
			# TODO: Create 2's complement
			s = int(s,16)
			if s > (((2**(l*8))-1)/2):
				s -= (1<<((l*8)-1))
		else:
			s = int(s,16)

		#print("Entered ",bytes,"whith converter %d " %converter, " got out", s)
		return s

	def printResults(self, result):
		addr = result[0]
		dataLen = result[1]
		dataRes = result[2]
		#print(addr,dataLen,dataRes)
		if addr in self.data.keys():
			for tup in self.data[addr]["c"]:
				convertedData = self.convertData(dataRes[tup[0]:tup[0]+tup[1]],tup[2])
				print convertedData

	def run(self):
		while(self.running):
			#print("PARSER: RUN")
			#print(self.q.qsize())
			try:
				while True:
					line = self.q.get(0)
					if(len(line) != 0):
						result = self.canDbgParse(line)
						self.printResults(result)
			except Queue.Empty:
				pass
			sleep(0.001)


def initData(data):
	graphCount = 0
	print("This is what we gonna listen")
	print("============================")
	for key in data.keys():
		# results
		data[key]["r"] = []
		for st,le,co in data[key]["c"]:
			print(key,st,le,co)
			data[key]["r"].append([0]*NPTSMAX)
			graphCount += 1
	print("============================")
	return graphCount

def main():
	isFile = False
	if isFile:
		path = "/Users/clemi/Work/canonair/mbed/Captures/PeugeotIdle-new.txt"
		speed = None
	else:
		path = "/dev/tty.usbmodem1412"
		speed = 230400
	dataQueue = Queue.Queue()

	#data = {"305":{"c":[(0,1,NONE),(1,1,NONE),(2,1,REVERSED)]},"208":{"c":[(4,2,TWOCOMPL)]}}
	#data = {"305":{"c":[(0,1,NONE),(1,1,NONE),(2,1,TWOCOMPL)]}}
	data = {"208":{"c":[(0,2,NONE)]},"3cd":{"c":[(5,2,NONE)]}}
	initData(data)

	reader = SerialReader(dataQueue,path,speed)
	cParsr = CanParser(dataQueue,data)

	reader.start()
	cParsr.start()

	while True:
		if select.select([sys.stdin,],[],[],0.0)[0]:
			break
		sleep(1)

	reader.stop()
	cParsr.stop()

if __name__ == '__main__':
	main()