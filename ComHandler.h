
#ifndef COM_HANDLER_H
#define COM_HANDLER_H

#include "MyThread.h"
#include "PyrnUSBModem.h"
#include "HTTPClient.h"
#include "MyCallBack.h"
#include "Configs.h"
#include "storageBase.h"
#include "sd.h"

#define XFER_BUFF_SZ 					512

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
	transferType tt;
	bool first;
	MyCallBack *cb;
	frameHdr hdr;
	Mutex BuffMtx;
	char *TXBuff;
	char RXBuff[XFER_BUFF_SZ];
	//char XFerBuff[XFER_BUFF_SZ];
	uint16_t currLen;
	uint16_t maxLen;
	HTTPClient http;
	int dataSz;
	ramCircBuff dataStorage;
	//sdCircBuff dataStorage;
public:
	ComHandler(MyCallBack *callback, 
				const char*idProduct, 
				ComHandler::transferType ltt = TT_HALF, 
				unsigned char *sp = NULL,
				char *pTXBuff = NULL, 
				uint16_t maxBuff = 0);
	virtual void SetTransferType(ComHandler::transferType ltt);
	virtual bool NeedTransfer(void);
	virtual void Main(void);
	virtual void DoServerRequest(void);
	virtual bool AddResults(uint8_t SensorType, char *data, uint16_t len);
	virtual void FillHeader();
};


#endif
