#! /usr/bin/python

import httplib

print("test")

token = "neo3cxKFqUuYxHZoJ5w4NA"
addr = "online-live1.services.u-blox.com"
url = "/GetOnlineData.ashx?token=%s;gnss=gps,glo;datatype=eph,alm,aux;"%token

conn = httplib.HTTPConnection(addr)
conn.request("GET", url);
r1 = conn.getresponse()
print r1.status, r1.reason
data1 = r1.read()
file = open("alm.bin","wb")
file.write(data1)
file.close()
#print(data1)
conn.close()