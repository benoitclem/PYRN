
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
public:
	typedef enum _transferType {
		TT_ASAP,
		TT_HALF,
		TT_FULL,
		TT_TWICE,
	} transferType;
protected:
typedef struct _frameHdr {
	char imei[15];
	char idCfg[40];
	uint32_t time;
	char newSess;
	//char cframe; // CurrentFrame
	//char tframe; // TotalFrame
	}__attribute__((packed)) frameHdr;
// This enum define how 
	transferType tt;
	bool first;
	MyCallBack *cb;
	frameHdr hdr;
	Mutex BuffMtx;
	char *TXBuff;
	char data[XFER_BUFF_SZ];
	char storageName[32];
	//char XFerBuff[XFER_BUFF_SZ];
	uint16_t currLen;
	uint16_t maxLen;
   	HTTPClient http;
public:
	ComHandler(MyCallBack *callback, const char*idProduct, ComHandler::transferType ltt = TT_HALF, const char *storNme = "impacts", char *pTXBuff = NULL, uint16_t maxBuff = 0);
	
	virtual void SetTransferType(ComHandler::transferType ltt);
	virtual bool NeedTransfer(void);
	virtual void Main(void);
	virtual void DoServerRequest(void);
	virtual bool AddResults(uint8_t SensorType, char *data, uint16_t len);
	virtual void FillHeader();
};


#endif
