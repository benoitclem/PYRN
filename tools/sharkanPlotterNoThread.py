#!/usr/bin/env python

import pylab
from pylab import *
from serial import Serial
import Queue
import time
import sys
import select
import math

NONE 	 = 0x00
REVERSED = 0x01
TWOCOMPL = 0x02

NPTSMAX  = 1000
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
		line, self.buf = self.buf, ''
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

class SerialReader:
	def __init__(self,queue,path,speed = None ):
		#print("sr: init")
		self.q = queue
		if (path[0:8] == "/dev/tty") and (speed != None):
			print("Data Source is serial port %s @%d" %(path,speed))
			self.dataSource = EnhancedSerial('/dev/ttyACM0', speed)
		else:
			print("Data Source is File %s" %path)
			self.dataSource = open(path,'r')

	def run(self,arg):
		#print("READER: RUN")
		v = self.dataSource.readline()
		self.q.put(v) 

class CanParser:
	def __init__(self, queue, data):
		self.q = queue
		self.data = data

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

	def storeResults(self, result):
		addr = result[0]
		dataLen = result[1]
		dataRes = result[2]
		changed = False
		if addr in self.data.keys():
			changed = True
			i = 0
			for tup in self.data[addr]["c"]:
				convertedData = self.convertData(dataRes[tup[0]:tup[0]+tup[1]],tup[2])
				self.data[addr]["r"][i].append(convertedData)
				i += 1
		return changed

	def plotData(self):
		keys = self.data.keys()
		for key in keys:
			indexPlot = 0
			for values in self.data[key]["r"]:
				if len(values):
					ax = self.data[key]["v"][(indexPlot*2)+0]
					line = self.data[key]["v"][(indexPlot*2)+1]
					
					if(len(values) > NPTSMAX):
						values = values[-NPTSMAX:]

					CurrentXAxis = pylab.arange(0,len(values),1)
					#print(len(CurrentXAxis),len(values))
				  	line[0].set_data(CurrentXAxis,pylab.array(values))
				  	indexPlot += 1

	def run(self,arg):
		changed = 0
		#print("PARSER: RUN")
		#print(self.q.qsize())
		try:
			while True:
				line = self.q.get(0)
				if(len(line) != 0):
					result = self.canDbgParse(line)
					if self.storeResults(result):
						changed +=1
		except Queue.Empty:
			pass

		if changed:
			#print("CHANGED")
			self.plotData()
			#measureTime(self.plotData,'plot')


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
	

def initVisu(data,graphCount):
	currGraph = 1
	fig = pylab.figure(1)
	for key in data.keys():
		# visualisation
		data[key]["v"] = []
		for st,le,co in data[key]["c"]:
			# subplot line col index
			nlines = int(math.ceil(float(graphCount) / float(NCOLMAX)))
			subGraph = nlines * 100
			subGraph += NCOLMAX * 10
			subGraph += currGraph
			#print(subGraph)
			# ax
			ax = fig.add_subplot(subGraph)
			ax.grid(True)
			ax.set_title("%s - %d"%(key,st))
			mini = 0
			maxi = 2**(8*le)
			if co & TWOCOMPL:
				maxi = maxi >> 1
				mini = -maxi
			print(maxi,mini)
			ax.axis([0,NPTSMAX,mini,maxi])
			data[key]["v"].append(ax)
			# linespace
			xAchse=pylab.arange(0,NPTSMAX,1)
			yAchse=pylab.array([0]*NPTSMAX)
			line=ax.plot(xAchse,yAchse,'-')
			data[key]["v"].append(line)

			currGraph += 1
	return fig

def commiter(arg):
	#print(arg)
	manager = arg;
	manager.canvas.draw()

def main():
	isFile = True
	if isFile:
		path = "/Users/clemi/Work/canonair/mbed/Captures/PeugeotIdle-new.txt"
		speed = None
	else:
		path = "/dev/ttyACM0"
		speed = 230400
	dataQueue = Queue.Queue()

	#data = {"305":{"c":[(0,1,NONE),(1,1,NONE),(2,1,REVERSED)]},"208":{"c":[(4,2,TWOCOMPL)]}}
	data = {"305":{"c":[(0,1,NONE),(1,1,NONE),(2,1,TWOCOMPL)]}}
	gc = initData(data)
	fig = initVisu(data,gc)
	manager = pylab.get_current_fig_manager()

	#print(data)

	reader = SerialReader(dataQueue,path,speed)
	cParsr = CanParser(dataQueue,data)

	readerTimer = fig.canvas.new_timer(interval=0.01)
	readerTimer.add_callback(reader.run, ())
	
	parserTimer = fig.canvas.new_timer(interval=2)
	parserTimer.add_callback(cParsr.run, ())

	commitTimer = fig.canvas.new_timer(interval= 1000)
	commitTimer.add_callback(commiter,(manager))
	
	readerTimer.start()
	parserTimer.start()
	commitTimer.start()

	pylab.show()
	


if __name__ == '__main__':
	main()