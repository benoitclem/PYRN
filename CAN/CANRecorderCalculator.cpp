#include "CANRecorderCalculator.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "CANRecorderCalculator.cpp"
#endif
#include "MyDebug.h"


CANRecorderCalculator::CANRecorderCalculator(const char *buff) {
	SetData(buff);
}

int CANRecorderCalculator::SetData(const char *buff) {
	int v = 0;
	if(buff) {
		v = Parse(buff);
		Validate();
	}
	return v;
}

int CANRecorderCalculator::Parse(const char* buff) {
	// Set the flags
	dataPresence = false;
	dataValidity = false;
	
	DBG("Calculator Recorder Parsing - HdrType(%d)", buff[0]); 
	if(buff[0] == CAN_RECORDER_HDR_TYPE) {
		memcpy(&calc,buff,sizeof(CANRecorderCalculatorHeader));
		DBG_MEMDUMP("RecordHdrData",(const char*)&calc,sizeof(CANRecorderCalculatorHeader));
		if(calc.nChunks>0) {
			if((calc.nChunks) < CAN_RECORDER_ENTRY_STORAGE) {
				memcpy(recordData,buff+sizeof(CANRecorderCalculatorHeader),(calc.nChunks*sizeof(CANRecorderData)));
				recordEntries = (CANRecorderData*)recordData;
				//DBG_MEMDUMP("DiagData",(const char*)cmd,calc.cmdLen);
				dataPresence = true;
				return sizeof(CANRecorderCalculatorHeader) + (calc.nChunks*sizeof(CANRecorderData)) ;
			} else {
				ERR("STORAGE is not big enough to record all chunks");
			}
		} else {
			ERR("No chunks or invalid value");
		}
	}
	return -1;
}

bool CANRecorderCalculator::Validate(void){
	dataValidity = false;
	if(dataPresence) {
		dataValidity = true;
		uint8_t s = 0;
		uint8_t l = 0;
		uint8_t i = 0;
		for( ;i<calc.nChunks; i++) {
			s = recordEntries[i].start;
			l = recordEntries[i].len;
			DBG("S = %d | L = %d",s,l);
			// S 0-7
			// L 1-8
			if(s>7) {
				dataValidity = false;
				ERR("Wrong record start point = %d",s);
				return false;
			}
			if((s+l)>8) {
				dataValidity = false;
				ERR("Wrong record len = %d->%d",s,s+(l-1));
				return false;
			}
		}
		INFO("Recorder Calculator Validation: Everything goes right");
		INFO("I've validated %d cmds",i);
		dataValidity = true;
		return true;
	}
	return false;
}

CANRecorderData * CANRecorderCalculator::GetRecordEntry(uint8_t index) {
	if(index < calc.nChunks) {
		return recordEntries+index;
	} else {
		return NULL;
	}
}