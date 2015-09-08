#include "CANDiagCalculator.h"

#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "CANDiagCalculator.cpp"
#endif
#include "MyDebug.h"

#define CAN_DIAG_HDR_VER 		1

extern MyMemoryAllocator memAlloc;

CANDiagCalculator::CANDiagCalculator(const char *buff) {
	cmd = NULL;
	SetData(buff);
}

int CANDiagCalculator::SetData(const char *buff) {
	int v = 0;
	if(buff) {
		v = Parse(buff);
		Validate();
	}
	return v;
}

int CANDiagCalculator::Parse(const char* buff) {
	// Set the flags
	dataPresence = false;
	dataValidity = false;
	
	DBG("Calculator Parsing - HdrVersion(%d)", buff[0]); 
	if(buff[0] == CAN_DIAG_HDR_VER) {

		memcpy(&calc,buff,sizeof(CANDiagCalculatorHeader));
		//DBG_MEMDUMP("HdrData",(const char*)&calc,sizeof(CANDiagCalculatorHeader));
		
		if(calc.cmdLen>0) {
			if(calc.cmdLen < CAN_DIAG_CMD_STORAGE) {
				memcpy(cmdStorage,buff+sizeof(CANDiagCalculatorHeader),calc.cmdLen);
				cmd = (CANDiagCommandHeader*)cmdStorage;
				//DBG_MEMDUMP("DiagData",(const char*)cmd,calc.cmdLen);
				dataPresence = true;
				return calc.cmdLen + sizeof(CANDiagCalculatorHeader);
			} else {
				ERR("STORAGE is not big enough to record all CMDs");
			}
		} else {
			ERR("No Commands or cmdLen invalid");
		}
	}
	return -1;
}

bool CANDiagCalculator::Validate(void){
	if(dataPresence) {
		uint16_t i = 0;
		uint8_t s = 0;
		uint16_t t = 0;
		char *pCmdHdr = (char*)cmd;
		dataValidity = true;
		while(1) {
			t = ((CANDiagCommandHeader*)pCmdHdr)->type;
			s = ((CANDiagCommandHeader*)pCmdHdr)->size;
			DBG("T = %d | S = %d",t,s);
			if(t == CMD_END) {
				break;
			} else if(s>8) {
				dataValidity = false;
				ERR("Wrong cmd size = %d",s);
				return false;
			}
			pCmdHdr += (sizeof(CANDiagCommandHeader) + s);
			i++;
		}
		INFO("Calculator Validation: Everything goes right");
		INFO("I've validated %d cmds",i);
		dataValidity = true;
		return true;
	}
	return false;
}

bool CANDiagCalculator::Ready(void) {
	return (dataPresence and dataValidity);
}

void CANDiagCalculator::FirstCommand(CANDiagCommandHeader **pCmd, uint16_t cmdType) {
	char *pCmdHdr = (char*)cmd;
	//DBG("Look for 1st Command @ %p",pCmdHdr);
	//DBG_MEMDUMP("CMDData",pCmdHdr,calc->cmdLen);
	// Check the first 
	uint16_t t = ((CANDiagCommandHeader*)pCmdHdr)->type;
	uint8_t s = ((CANDiagCommandHeader*)pCmdHdr)->size;
	//DBG("type %02x - %02x",cmdType,t);
	if(t == cmdType) {
		*pCmd = (CANDiagCommandHeader*)pCmdHdr;		
		return;
	}
	//DBG("Look for 1st Command 0");
	// Iterate over the next Cmds to see if we got a result and return anyway
	NextCommand((CANDiagCommandHeader**)&pCmdHdr,cmdType);
	*pCmd = (CANDiagCommandHeader*)pCmdHdr;
	//DBG("Look for 1st Command 1 %p", *pCmd);
}

void CANDiagCalculator::NextCommand(CANDiagCommandHeader **pCmd, uint16_t cmdType) {
	char *pCmdHdr = (char*)*pCmd;
	uint16_t t; 
	uint8_t  s = ((CANDiagCommandHeader*)pCmdHdr)->size;
	while(1) {
		//DBG("Next Command");
		// Go Next
		//DBG("- %d %d %d",(sizeof(CANDiagCommandHeader) + s), sizeof(CANDiagCommandHeader),s);
		pCmdHdr += (sizeof(CANDiagCommandHeader) + s);
		if(pCmdHdr) {
			t = ((CANDiagCommandHeader*)pCmdHdr)->type;
			s = ((CANDiagCommandHeader*)pCmdHdr)->size;
			//DBG("type %02x - %02x",cmdType,t);
			if(t == cmdType) {
				*pCmd = (CANDiagCommandHeader*) pCmdHdr;
				//DBG("Matched %p",*pCmd);
				return;
			} else if( t == CMD_END) {
				break;	
			}
		} else {
			break;		
		}
	}
	*pCmd = NULL;
}

