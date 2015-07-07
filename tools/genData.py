#!/usr/bin/python

import urllib2
from struct import *
from time import time

url = "http://192.168.1.200:8080/index2.php"
#imei = "012345678901234"
imei = "AAAAAAAAAAAAAAA"
idCfg = "0123456789/123456789/123456789/123456789"
tmstp = int(time())
newSes = "1"

tpacked = pack('<L', tmstp)

s = imei+idCfg+tpacked+newSes

print(s)

#print(type(s))
#print(s)

u = urllib2.urlopen(url, s)

l = u.read()

for c in l:
	print(hex(ord(c)))

arr = unpack('<cich',l[:8])

hdrver = arr[0]
idCalc = arr[1]
diagCd = arr[2]
speed = arr[3]

print(hdrver,idCalc,diagCd,speed)


