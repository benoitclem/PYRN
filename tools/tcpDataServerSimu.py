#! /usr/bin/python
#coding: utf-8

import sys
import json
import random
import signal
from time import sleep
from struct import *
from math import sin

def signal_handler(signal, frame):
    print 'You pressed Ctrl+C!'
    sys.exit(0)
 
signal.signal(signal.SIGINT, signal_handler)

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)

dt = 1.0
if len(sys.argv) == 2:
    dt = float(sys.argv[1])
    print("Argment dt :")
else:
    print("no time argument take dt :")

print(dt)

while True:
    calcs = [{"rxAdd":0x608,"chunks":[(2,2),(3,1)]},{"rxAdd":0x5bb,"chunks":[(4,2)]},{"rxAdd":0x12e,"chunks":[(0,1),(3,1),(4,1)]}] 
    superDic = []

    conf = pack('=B',len(calcs)) 
    for calc in calcs:
        conf += pack("=IB",calc["rxAdd"],len(calc["chunks"]))
        for chunk in calc["chunks"]:
            conf += pack("=BB",chunk[0],chunk[1])

    for c in conf:
        print(format(ord(c),"02x")),
        #print hex(c),
    print("")
    for calc in calcs:
        jDic = {"addr":calc["rxAdd"]}
        jDic["chunks"] = calc["chunks"]
        jDic["datas"] = []
        for chunk in calc["chunks"]:
            jDic["datas"].append([])
        i = 0
        for chunk in calc["chunks"]:
            jDic["datas"][i%len(calc["chunks"])].append(''.join(random.choice('0123456789abcdef') for n in xrange(chunk[1]*2)))
            i += 1
        superDic.append(jDic)

    print(superDic)   
    jData = json.dumps(superDic)
    f = open('workfile', 'w')
    f.write(jData)
    f.close()

    sleep(dt)
