
#include "CANCommunicator68.h"
#include "CANDiagCalculator.h"
#include "CANCommon.h"
#include "MyCallBackIds.h"
#include "MyOsHelpers.h"
#include "Configs.h"

#define __DEBUG__ CAN_COMM_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANCommunicator68.cpp"
#endif
#include "MyDebug.h"


// The MASTER Stuffs

CANCommunicator68::CANCommunicator68(CANInterface *can, uint8_t bus, CANDiagCalculator *c):
	CANCommunicatorMaster(can,bus,c) {
}

// There is a weird thing here, when I send the cmd before adding the callback id to listen
// There is a lot of chance to miss the answer, this is true when simulation (local loop), 
// don't know in real life. 

void CANCommunicator68::ExecuteCommand(char *data, uint8_t len, char *response, uint8_t *respLen, uint32_t msTmo) {

	osEvent evt;

	char container[8];
	uint8_t currLen = 0;

	if(respLen != NULL)
		*respLen = 0;
	else
		return;

	CANDiagCommandHeader *hdrCmd = NULL;
	DBG("bus %d - calc %p", bus, calc);
	if((bus != 0) && (calc != NULL)) {
		// Flush queue
		FlushQueue();
		// Tell we want receive the event now
		canItf->AddCallBackForId(bus, calc->GetAddrDst(), this);
		// Put data in container to create padding
		memset(container,0,8);
		memcpy(container,data,len);
		// Multiple lines is not handled for the moment
		DBG_MEMDUMP("(68)ExecuteCommand - Send CMD",container,8);
		//DBG("Sending %04x",calc->GetAddrSrc());
		canItf->Send(bus,calc->GetAddrSrc(),container,8);
		// If no Response and no timeout and no respLen pointer do nothing
		if((msTmo != 0) && (response != NULL) && (respLen != NULL)) {
			// Wait for answer
			evt = queue.get(msTmo);
			if(evt.status == osEventMessage) {
				CANMessage *msg = (CANMessage*) evt.value.p;
				DBG("(68)ExecuteCommand - Received Response");
				*respLen = msg->data[0];
				memcpy(response,msg->data+1,*respLen);
				delete(msg);
			} else if (evt.status == osEventTimeout) {
				//PrintActiveThreads();
				DBG("(68)ExecuteCommand - failed (timeout)");
			} else {
				DBG("(68)Other type");
			}
		}
		// read queue
		canItf->DelCallBackForId(bus, calc->GetAddrDst());
	}
}

