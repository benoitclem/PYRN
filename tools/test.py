#!/usr/bin/env python
#coding: utf-8

import matplotlib.pyplot as plt
from pylab import *

#f = open("/Users/clemi/Work/canonair/mbed/Captures/Capt41.txt")
#f = open("/Users/clemi/Work/canonair/mbed/Captures/407-3janv/ligneDroitevolantpassagevitesse.txt")
f = open("/Users/clemi/Work/canonair/mbed/Captures/407-3janv/ceinturejeux.txt")
v = f.read()
f.close()

v = v.split("\n")
addrs = {"208":{"c":[(0,2,0),(4,1,0),(7,1,0)],"r":[[],[],[]],"v":["g"]}}

"""
addrs = {"305":{"c":[(0,2,2)],"r":[[]],"v":["b"]}\
		,"208":{"c":[(0,2,0),(7,1,0)],"r":[[],[]],"v":["g"]}\
		,"348":{"c":[(0,1,0)],"r":[[]],"v":["b"]}\
		,"44d":{"c":[(0,2,0),(2,2,0),(4,2,0),(6,2,0)],"r":[[],[],[],[]],"v":["b"]}}
"""
"""
,"612":{"c":[(3,1,0)],"r":[[]],"v":["k"]}\
,"3cd":{"c":[(5,2,0)],"r":[[]],"v":["r"]}\
,"348":{"c":[(0,1,0)],"r":[[]],"v":["b"]}}
"""

t = 1000.0

def twos_comp(val, bits):
    """compute the 2's compliment of int value val"""
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val    

for l in v:
	cl = l.strip().split("|")
	addr = cl[1]
	dataLen = cl[2]
	data = cl[3].split(" ")
	#print(addr,dataLen,data)
	
	'''
	if addr not in arr.keys():
		arr[addr] = [0]*int(dataLen)
	
	i = 0
	for d in data:
		print(arr[addr])
		arr[addr][i].append(int(d,16))
		i += 1

	'''
	#"""
	if addr in addrs.keys():
		#print(addr,dataLen,data)

		chunks = addrs[addr]["c"]
		results = addrs[addr]["r"]
		i = 0
		for start,sz,co in chunks:
			d = data[start:start+sz]
			val = ""
			for c in d:
				val += c
			#print val
			# GO TO INT
			#print val
			s = 0
			if co == 2:
				s = twos_comp(int(val,16),sz*8)
			else:
				s = int(val,16)
			#print("Entered ",d,"whith converter %d " %co, " got out", s)
			results[i].append(s)
			i+=1
	#"""

#print(addrs)
nGraphs = 0
print(len(addrs))
for addr in addrs.keys():
	for chunks in addrs[addr]["c"]:
		nGraphs += 1
print nGraphs

fig = plt.figure(1)
fig.patch.set_facecolor('#FFFFFF')
iGraph = 1
for addr in addrs.keys():
	idx = 0
	for results in addrs[addr]["r"]:
		graphIndex = nGraphs * 100 + 10 + iGraph
		plt.subplot(graphIndex)
		dt = t / len(results)
		lineSpace = arange(0.0, t, dt)
		if(len(lineSpace)<len(results)):
			results = results[-len(lineSpace):]
		else:
			lineSpace = lineSpace[-len(results):]
		for visu in addrs[addr]["v"]:
			plt.plot(lineSpace, results,visu)
			#plt.plot(results,visu)
		plt.ylabel("%s - %d" %(addr,addrs[addr]["c"][idx][0]))
		idx += 1
		iGraph += 1

plt.show()

