
#ifndef COM_HANDLER_H
#define COM_HANDLER_H

#include "MyThread.h"
#include "PyrnUSBModem.h"
#include "HTTPClient.h"
#include "MyCallBack.h"

#define XFER_BUFF_SZ 512

class ResultHandler {
protected:
	char buff[XFER_BUFF_SZ];
	bool full;
public:
	ResultHandler() {full = false;};
	bool IsFull(void) {return full;};
	void SetFull() {full = true;};
	void getData(char *d, uint8_t sz) {};
	char *getDataPointer() {return buff;};
};

typedef struct _SizedData {
	uint16_t sz;
	char data[];
} SizedData;

class ComHandler: public MyThread {
protected:
typedef struct _frameHdr {
	char imei[15];
	char idCfg[40];
	uint32_t time;
	char newSess;
}__attribute__((packed)) frameHdr;
	bool first;
	MyCallBack *cb;
	frameHdr hdr;
	Mutex BuffMtx;
	char *TXBuff;
	//char XFerBuff[XFER_BUFF_SZ];
	uint16_t currLen;
	uint16_t maxLen;
   	HTTPClient http;
public:
	ComHandler(MyCallBack *callback, const char*idProduct, char *pTXBuff, uint16_t maxBuff);
	virtual void Main(void);
	virtual void DoServerRequest(void);
	virtual bool AddResults(uint8_t SensorType, char *data, uint16_t len);
	virtual void FillHeader();
};


#endif
