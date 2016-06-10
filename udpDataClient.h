
#ifndef UDP_DATA_CLIENT_H
#define UDP_DATA_CLIENT_H

#include "mbed.h"
#include "MyThread.h"
#include "UDPSocket.h"
#include "Endpoint.h"

class udpDataClient{
protected:
	UDPSocket socket;
	Endpoint server;
public:
	udpDataClient();
	virtual void SendData(char *data,uint16_t len);
	virtual int GetTestIDs(char *data,uint16_t len);
	virtual int GetConfig(uint8_t index, char *data,uint16_t len);
};
#endif