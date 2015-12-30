
#ifndef TCP_DATA_CLIENT_H
#define TCP_DATA_CLIENT_H

#include "mbed.h"
#include "MyThread.h"
#include "TCPSocketConnection.h"

class tcpDataClient: public MyThread {
protected:
	TCPSocketConnection socket;
public:
	tcpDataClient(unsigned char *sp = NULL, unsigned int sSz = 0);
	virtual bool isConnected(void);
	virtual void Main(void);
	virtual void SendData(char *data,uint16_t len);
	virtual int GetConfig(char *data,uint16_t len);
};
#endif