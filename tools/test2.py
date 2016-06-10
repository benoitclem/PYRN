#!/usr/bin/env python
#coding: utf-8

import operator
import matplotlib.pyplot as plt
from pylab import *

#f = open("/Users/clemi/Work/canonair/mbed/Captures/Capt41.txt")
#f = open("/Users/clemi/Work/canonair/mbed/Captures/407-3janv/ligneDroitevolantpassagevitesse.txt")
f = open("/Users/clemi/Work/canonair/mbed/Captures/407-3janv/ceinturejeux.txt")
v = f.read()
f.close()

v = v.split("\n")

t = 1000.0

nLines = 0

addrs = {}

def getStats(arr):
	items = {}
	changes = 0
	summ = 0
	state = -1
	for v in arr:
		summ += 1

		# Calculate changes
		if state == -1:
			state = v
		elif state != v:
			changes += 1
			state = v

		# calculate occurences
		if v in items.keys():
			items[v] += 1
		else:
			items[v] = 1
	return items,summ,changes

for l in v:
	cl = l.strip().split("|")
	addr = cl[1]
	dataLen = cl[2]
	data = cl[3].split(" ")
	#print(addr,dataLen,data)
	nLines += 1
	if addr not in addrs.keys():
		addrs[addr] = [1,int(dataLen),[]]
		for d in data:
			val = int(d,16)
			for i in range(8):
				bit = 0
				if(val&(1<<(7-i))):
					bit = 1
				addrs[addr][2].append([bit])
	else:
		addrs[addr][0] += 1
		i = 0
		for d in data:
			val = int(d,16)
			for j in range(8):
				bit = 0
				if(val&(1<<(7-j))):
					bit = 1
				addrs[addr][2][i].append(bit)
				i += 1

f = 0
for key in addrs.keys():
	print("---%s----"%key)
	n = 1
	for datas in addrs[key][2]:
		occArr,summ,changes = getStats(datas)
		if changes<7 and changes > 3:
			print(n,occArr,summ,changes)
			fig = plt.figure(f)
			plt.plot(datas)
			plt.ylabel("%s - %d - %d" %(key,n/8,n%8))
			f += 1
		n += 1

plt.show()

"""
for key in addrs.keys():
	for datas in addrs[key][2]:
		occArr,summ,changes = getStats(datas)
		nOcc = len(occArr)
		if nOcc < 5 and nOcc != 1:
			print("---------")
			#print(datas)
			print(occArr)
"""

