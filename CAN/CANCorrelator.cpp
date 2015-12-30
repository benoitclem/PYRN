
#include "CANCorrelator.h"

#include "CAN.h"
#include "CANFifoMessage.h"
#include "CANCommon.h"
#include "MyCallBackIds.h"
#include "Configs.h"

#define __DEBUG__ CAN_CORRELATOR_DBG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANCorrelator.cpp"
#endif
#include "MyDebug.h"


#define CORRELATION_FACTOR_MIN 0.5


CANCorrelator::CANCorrelator(CANInterface *itf, int bus) {
	canItf = itf;
	curFrames = 0;
	maxFrames = CAN_MAX_FRAMES;
	curVariables = 0;
	maxVariables = CAN_MAX_VARS;
	if(bus == 0) {
		itf->AddCallBackForId(CAN_BUS_DONT_CARE, CAN_ID_PROMISCUOUS_MODE, this);
	} else {
		itf->AddCallBackForId(bus,CAN_ID_PROMISCUOUS_MODE,this);
	}
}

float CANCorrelator::BasicCorrelation(char len, char *dataOne, char *dataTwo) {
	float corr = 0;
	for(int i = 0; i<len; i++) {
		if(dataOne[i] == dataTwo[i]) {
			corr++;
		}
	}
	corr /= (float)len;
	return corr;
}

int CANCorrelator::SearchSimilarFrame(int id, char len, char *data) {
	// First look if the id and len are identical
	for(int i = 0; i<curFrames; i++) {
		frameEntry *fe = frameList+i;
		if((fe->id == id) && (fe->len == len)) {
			float corrFactor = BasicCorrelation(len,data,fe->data);
			if(corrFactor>CORRELATION_FACTOR_MIN){
				return i;
			} 
		}
	}
	return -1;
}

int CANCorrelator::SearchSimilarVariable(int id, int index, int len) {
	for(int i = 0; i<curVariables; i++) {
		variableEntry *ve = variableList+i;
		//DBG("Search Similar: [%d %d %d]==[%d %d %d]?", ve->id, ve->index, ve->len, id, index, len);
		if((ve->id == id) && (ve->index == index) && (ve->len == len)) {
			return i;
		}
	}
	return -1;
}

int CANCorrelator::InsertNewVariable(int id, int s, int c) {
	variableList[curVariables].id = id;
	variableList[curVariables].index = s;
	variableList[curVariables].len = c;
	variableList[curVariables].vData = new CANVariableData(id,128,c);
	curVariables++;
	return curVariables-1; // return the newly inserted index
} 

void CANCorrelator::TrackVariations(int id, char len, char *dataOne, char *dataTwo) {
	char varBlock[16];
	int s = -1;
	int e = -1;
	int c = 0;
	int i = 0;
	for(; i < len; i++) {
		if(dataOne[i] != dataTwo[i]) {
			if(s == -1) {
				s = i;
			}
			varBlock[c++] = dataOne[i];
		} else if(c != 0){
			e = i;
			//char str[16];
			//sprintf(str,"[%d %d %d]",s,e,c);
			//DBG_MEMDUMP(str,varBlock,c);
			// Find corresponding frame in variable list
			int index = SearchSimilarVariable(id,s,c);
			// Store this result
			if(index >= 0) {
				// Fill the variable data in list
				DBG("Detected variable to be tracked(%d) [id:%08x s:%d l:%d]",index,id,s,c);
			} else {
				DBG("Could not find variable in list insert new variable to be tracked [id:%08x s:%d l:%d]",id,s,c);
				index = InsertNewVariable(id,s,c);
			}
			if(variableList[index].vData != NULL) { 
				variableList[index].vData->AddData(varBlock,c);
			} else { 
				ERR("Couldn't Reach variable list index");
			}
			s = -1;
			e = -1;
			c = 0;
		}
	}
	if(c!=0) {
		e = i;
		//char str[16];
		//sprintf(str,"[%d %d %d]",s,e,c);
		//DBG_MEMDUMP(str,varBlock,c);
		// Find corresponding frame in variable list
		int index = SearchSimilarVariable(id,s,c);
		// Store this result
		if(index > 0) {
			// Fill the variable data in list
			DBG("Detected variable to be tracked(%d) [id:%08x s:%d l:%d]",index,id,s,c);
		} else {
			DBG("Could not find variable in list insert new variable to be tracked [id:%08x s:%d l:%d]",id,s,c);
			index = InsertNewVariable(id,s,c);
		}
		if(variableList[index].vData != NULL) { 
			variableList[index].vData->AddData(varBlock,c);
		} else { 
			ERR("Couldn't Reach variable list index");
		}
	}
}

void CANCorrelator::PrintVariables() {
	DBG("==================================");
	for(int i = 0; i<curVariables; i++) {
		variableEntry *ve = variableList+i;
		DBG("(%08x)[%d %d %d]", i, ve->id, ve->index, ve->len);
		char *d = ve->vData->GetData();
		int s = ve->vData->GetDataSz();
		DBG_MEMDUMP("data:",d,s);
	}
	DBG("==================================");
}

void CANCorrelator::event(int ID, void *data) {
    //char buff[23];
    if((ID&0x0f) == CAN_CALLBACK) {
		uint8_t bus = (ID>>8) & 0x0f;
        // Remember we don't own the CANMessage here.
        CANMessage *msg = (CANMessage*) data;
        int id = msg->id;
        int len = msg->len;
        char *canData = (char*)msg->data;
        //CANPrintWorkAround("CORRELATOR RX",bus,id,canData,len); 
        char str[20];
        //sprintf(str,"[%d|%08X|%d]",bus,id,len);
        //DBG_MEMDUMP(str,canData,len);
        if(len > CAN_FRAME_MAX_LEN) {
			DBG("Our System is not ready to read frames whith length > %d, you can maybe increase the MAX",CAN_FRAME_MAX_LEN);
		} else {
			int index = SearchSimilarFrame(id,len,canData);
			if(index >= 0) {
				//DBG("We found a match in our list %d",index);
				TrackVariations(id,len,canData,frameList[index].data);
				// Replace the current data by the received data
				memcpy(frameList[index].data,canData,len);
			} else {
				if(curFrames<=maxFrames) {
					DBG("Could not find a match in list... insert new");
					frameList[curFrames].id = id;
					frameList[curFrames].len = len;
					memcpy(frameList[curFrames].data,canData,len);
					curFrames++;
				} else {
					DBG("Could not find a match in list... but list is full");
				}
			}
		}
    }
}