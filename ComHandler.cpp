
#include "ComHandler.h"
#include "HTTPText.h"
#include "HTTPRawData.h"
#include "MyMemoryAllocator.h"
#include "Storage.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "ComHandler.cpp"
#endif
#include "MyDebug.h"

#define COM_HANDLER_BUFF_SIZE			2096
#define COM_HANDLER_THREAD_STACK_SIZE   4*1024
#define COM_HANDLER_MIN_PKTSZ			128

extern Storage 			*storage;
extern MyMemoryAllocator memAlloc;

ComHandler::ComHandler(MyCallBack *callback, const char* idProduct, ComHandler::transferType ltt, const char *storNme, char *pTXBuff, uint16_t maxBuff): MyThread("ComHandler",COM_HANDLER_THREAD_STACK_SIZE) {
	cb = callback;
	if(pTXBuff != NULL) {
		TXBuff = pTXBuff;
		maxLen = maxBuff;
	} else {
		TXBuff = (char*) memAlloc.malloc(COM_HANDLER_BUFF_SIZE);
		maxLen = COM_HANDLER_BUFF_SIZE;
	}
	tt = ltt;
	strcpy(storageName,storNme);
	memcpy(hdr.imei,idProduct,15);
	memset(hdr.idCfg,0xa5,40);

	first = true;
}

void ComHandler::SetTransferType(ComHandler::transferType ltt) {
	tt = ltt;
}

bool ComHandler::NeedTransfer(void) {
	uint16_t limit = 0;
	switch(tt) {
		case TT_ASAP:
			return true;
		case TT_HALF:
			limit = maxLen/2;
			break;
		case TT_FULL:
			limit = maxLen;
			break;
		case TT_TWICE:
			limit = maxLen*2;
	}
	return storage->ReadDataSize("impacts")>=limit;
}

void ComHandler::Main(void) {
	DBG("Running ComHandler");
    while(running) {
        DBG("ComHandler Loop");
        Thread::wait(1000);
		//if(true){		
		if(NeedTransfer()) { 
			DoServerRequest();
		} else {
			DBG("Not Enough data to send results");			
		}
    }
}

void ComHandler::DoServerRequest(void) {
	char hdrTmp[3];
	
	memset(data,0,XFER_BUFF_SZ);
	// Protect access to TXBuff	
	BuffMtx.lock();
	
	currLen = 0;
	
	// Create a Frame
	FillHeader();
	currLen += sizeof(frameHdr);
	
	// Fill up Frame with data from storage
	uint16_t s = 0;
	uint16_t r = 0;
	DBG("Now build a frame from storage");
	while(1) {
		// Read Impact Header to
		storage->ReadData("impacts", (uint8_t*)hdrTmp, 3, 0,false);
		DBG_MEMDUMP("HdrTmp", hdrTmp, 3);
		s = *((uint16_t*)(hdrTmp+1));
		DBG("Try to add %d char to %d/%d buff", s, currLen, maxLen);
		if((currLen+s)<maxLen) {
			r = storage->ReadData("impacts", (uint8_t*)(TXBuff+currLen), s+3, 0, true);
			if(r == 0) {
				DBG("Storage is now empty ... close frame");
				break;
			}
		} else {
			DBG("Reached Frame max size ... close frame");
			break;
		}
	}
	
    DBG("Reporting Results (%d) ...\n",currLen);
	DBG_MEMDUMP("TXData",(char*)TXBuff, currLen);
	// Create the exchange buffers (becarefull with thread stack)
	// /!\ WORKAROUND +1 (http client miss 1byte)
	HTTPRawData httpDataOut(TXBuff, currLen+1);
	HTTPRawData httpDataIn(data, XFER_BUFF_SZ);
	int ret = http.post("http://pyrn-m2m.zagett.com:50007/index2.php", httpDataOut, &httpDataIn);
	int sz = httpDataIn.getRcvdLen();
	//DBG("HTTP POST(%d) sz = %d",ret,sz);
    if (!ret) {
		//DBG_MEMDUMP("Page",data,sz);
		// TODO: Do a validation of data before stopping everything
		if((cb != NULL) && (sz != 0) ) {
    		cb->event(sz,(void*)data);
    		DBG("Overwrite the current calcs stored in SD");
    	} else{
    		DBG("Callback not found or size is null? (%d)",sz);
    	}
    	storage->WriteData("calcs",(uint8_t*)data,sz,true);
		//DBG("Page fetched succesfully - read %d characters\n", strlen(data));
		//DBG("Result: %s\n", data);
    } else {
		DBG("Error - ret = %d - HTTP return code = %d\n", ret, http.getHTTPResponseCode());
    }
	BuffMtx.unlock();
}

bool ComHandler::AddResults(uint8_t SensorType, char *data, uint16_t len) {
	BuffMtx.lock();
	TXBuff[0] = SensorType;
	TXBuff[1] = (char) (len) & 0xff;
	TXBuff[2] = (char) (len>>8) & 0xff;
	memcpy(TXBuff+3,data,len);
	int r = storage->WriteData("impacts",(uint8_t*)TXBuff,len+3,false);
	BuffMtx.unlock();
	return r==(len+3);
}

void ComHandler::FillHeader() {
	// idconfig to fetch
	// memcpy(hdr->idCfg,,10);
	hdr.time = time(NULL);
	hdr.newSess = (first)?'1':'0';
	if(first)
		first = false;
	memcpy(TXBuff,(char*)&hdr,sizeof(frameHdr));
}

