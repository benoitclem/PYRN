#! /usr/bin/python
#coding: utf-8

import socket
import signal
import sys
from struct import *
import json
import select
import time

def signal_handler(signal, frame):
    print 'You pressed Ctrl+C!'
    server.close()
    sys.exit(0)
 
signal.signal(signal.SIGINT, signal_handler)

def getTestId():

    # open multi file
    f = open('multiConfig', 'r')
    d = f.read()
    f.close()
    #print(d)

    calcGroups = json.loads(d)

    # Take first rxAd
    calcId = calcGroups[0][0]["rxAdd"]
    print("sending %d"%calcId)
    conf = pack("=I",calcId)
    return conf

def getConfig(index):

    # open file
    f = open('multiConfig', 'r')
    d = f.read()
    f.close()
    #print(d)

    calcs = json.loads(d)[unpack('=B',index)[0]]

    print(calcs)

    # put the number of calcs
    conf = pack('=B',len(calcs)) 
    for calc in calcs:
        # put the addr and n chunks for each calc
        conf += pack("=IB",calc["rxAdd"],len(calc["chunks"]))
        for chunk in calc["chunks"]:
            # put the chunks start and len
            conf += pack("=BB",chunk[0],chunk[1])

    return conf

def parseFrame(rdata):
    dec = 3
    datas = []
    while True:
        packLen = unpack('=H', rdata[dec:dec+2])[0]
        #print(dec,packLen,len(rdata))
        #print(rdata[dec+2:dec+packLen+1])
        #print(len(rdata[dec+2:dec+packLen+2]))
        datas.append(rdata[dec+2:dec+packLen+2])
        dec += packLen + 2
        if dec >= len(rdata):
            break

    superDic = []
    for data in datas:
        #print(len(data))
        (rxAdd,dt,sz,nChunks) = unpack('=IHHB', data[:9])

        if rxAdd == 0x5A5A5A5A:
            print("GPS")
            rxAdd = "GPS"
        if rxAdd == 0xA5A5A5A5:
            print("IMU")
            rxAdd = "IMU"

        #print(rxAdd,dt,sz,nChunks)
        chunks = []
        for i in range(nChunks):
            #print(9+(i*2),9+(i*2)+2)
            chunks.append(unpack('=BB',data[9+(i*2):9+(i*2)+2]))
        #print(chunks)
        rawData = data[9+nChunks*2:]
        #print(len(rawData))
        #print(rawData)
        jDic = {}
        jDic["addr"] = rxAdd
        jDic["chunks"] = chunks
        jDic["datas"] = []
        jDic["dt"] = time.time()

        for chunk in chunks:
            jDic["datas"].append([])

        i = 0
        j = 0
        nChunks = len(chunks)
        if nChunks != 0:
            while(i<len(rawData)):
                offset = chunks[j%nChunks][1]
                chunk = rawData[i:i+offset]
                hChunk = ""
                #hex(int(ord(t["datas"][1][0][0])))[2:]
                for c in chunk:
                    hChunk += format(ord(c),'02x')
                #print(j%nChunks,chunk)
                """if rxAdd == 0xA5A5A5A5:
                    # imu
                    print(len(hChunk))
                    if len(hChunk) == 48:
                        jDic["datas"][j%nChunks].append([hChunk[0:8],hChunk[8:16],hChunk[16:24],hChunk[24:32],hChunk[32:40],hChunk[40:48]])

                else:
                    """
                jDic["datas"][j%nChunks].append(hChunk)
                i += offset
                j += 1

            superDic.append(jDic)
    return superDic
 
print 'Server Running at ', socket.gethostbyname(socket.gethostname()) 
server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('', 6666))

timeout = 5.0

inputs = [server]
 
while True:
    readable, writable, exceptional = select.select(inputs, [], [], timeout)

    if not (readable or writable or exceptional):
        print("Got Timeout")
    else:
        for s in readable:
            rdata, addr = s.recvfrom(1024)
            print("RX Packet from", addr)

            if len(rdata) == 0:
                continue

            if ord(rdata[0]) == 0x00:  # testId wanted
                print("TestId")
                conf = getTestId()
                print(len(conf))
                s.sendto(conf,addr)
            if ord(rdata[0]) == 0x01:    # configuration wanted
                print("config")
                print(len(rdata))
                conf = getConfig(rdata[1])
                print(len(conf))
                s.sendto(conf,addr)
            elif ord(rdata[0]) == 0x02:    # frame array
                print("frame array")
                print(len(rdata))
                pktCount = unpack('=H', rdata[1:3])[0]
                print("pktcount: " + str(pktCount))
                try:
                    superDic = parseFrame(rdata)
                    print(superDic)
                    # record Data
                    jData = json.dumps(superDic)
                    f = open('workfile', 'w')
                    f.write(jData)
                    f.close()
                except:
                    print("got error")
#conn.close()
