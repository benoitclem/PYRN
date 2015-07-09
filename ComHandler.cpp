
#include "ComHandler.h"
#include "HTTPText.h"
#include "HTTPRawData.h"
#include "MyMemoryAllocator.h"

#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "ComHandler.cpp"
#endif
#include "MyDebug.h"

#define COM_HANDLER_BUFF_SIZE			4096
#define COM_HANDLER_THREAD_STACK_SIZE   3*1024
#define COM_HANDLER_MIN_PKTSZ			128

extern MyMemoryAllocator memAlloc;

ComHandler::ComHandler(MyCallBack *callback, const char* idProduct, char *pTXBuff, uint16_t maxBuff): MyThread("ComHandler",COM_HANDLER_THREAD_STACK_SIZE) {
	cb = callback;
	if(pTXBuff != NULL) {
		TXBuff = pTXBuff;
		maxLen = maxBuff;
	} else {
		TXBuff = (char*) memAlloc.malloc(COM_HANDLER_BUFF_SIZE);
		maxLen = COM_HANDLER_BUFF_SIZE;
	}
	currLen = sizeof(frameHdr);
	memcpy(hdr.imei,idProduct,15);
	memset(hdr.idCfg,0xa5,40);
	first = true;
}

void ComHandler::Main() {
	int loop = 0;

	DBG("Running ComHandler");
    while(running) {
        DBG("ComHandler Loop");
        Thread::wait(1000);
		if((loop%10) == 0) {
			if(true){			
			//if(currLen > COM_HANDLER_MIN_PKTSZ) {
				DoServerRequest();
			} else {
				DBG("Not Enough (%d) data to send results",currLen);			
			}
        }
        loop++;
    }
}

void ComHandler::DoServerRequest(void) {
	memset(data,0,XFER_BUFF_SZ);
	// Protect access to TXBuff	
	BuffMtx.lock();
	FillHeader();
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
    	} else{
    		DBG("Callback not found or size is null? (%d)",sz);
    	}
		//DBG("Page fetched succesfully - read %d characters\n", strlen(data));
		//DBG("Result: %s\n", data);
    } else {
		DBG("Error - ret = %d - HTTP return code = %d\n", ret, http.getHTTPResponseCode());
    }
	currLen = sizeof(frameHdr);
	BuffMtx.unlock();
}

bool ComHandler::AddResults(uint8_t SensorType, char *data, uint16_t len) {
	// TODO: Add timeout
	bool ret = false;
	if( (currLen + len + 2) < maxLen) {
		BuffMtx.lock();
		TXBuff[currLen++] = SensorType;
		TXBuff[currLen++] = (char) (len) & 0xff;
		TXBuff[currLen++] = (char) (len>>8) & 0xff;
		memcpy(TXBuff+currLen,data,len);
		currLen+=len;
		ret = true;
		DBG("ADD[%d] %d data now currLen = %d/%d",SensorType, len+3, currLen, maxLen);
		BuffMtx.unlock();
	} else {
		ERR("Could not fit Sensor data into output Buffer");	
	}
	return ret;
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

