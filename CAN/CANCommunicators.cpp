
#include "CANCommunicators.h"
#include "MyCallBackIds.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "CANCommunicators.cpp"
#endif
#include "MyDebug.h"

// ==================== BASE THINGS ====================

CANCommunicatorBase::CANCommunicatorBase(CANInterface *can,uint8_t b, CANDiagCalculator *c) {
	canItf = can;
	if(b) {
		bus = b;
		// idSrc = idS;
		// idDest = idD;
		calc = c;
	}
}

// Use this function to reconfigure the object over the AIR
void CANCommunicatorBase::Configure(uint8_t b, CANDiagCalculator *c) {
	if(b) {
		bus = b;
		// idSrc = idS;
		// idDest = idD;
		calc = c;
	}
}

// ==================== MASTER THINGS ====================

CANCommunicatorMaster::CANCommunicatorMaster(CANInterface *can, uint8_t bus, CANDiagCalculator *c): 
	CANCommunicatorBase(can,bus,c) {
}

void CANCommunicatorMaster::FlushQueue(void) {
	osEvent evt;
	while(true) {
		evt = queue.get(1000);
		if(evt.status != osEventMessage)
			break;
		else {
			CANMessage *msg = (CANMessage*) evt.value.p;
			delete(msg);
		}
	}
}

void CANCommunicatorMaster::event(int ID, void *data) {
	// Check the callback is well destinated
	if((ID&0x0f) == CAN_CALLBACK){
		CANMessage *msgCp = new CANMessage;
		CANMessage *msg = (CANMessage*) data;
		// The callback don't own the CANMessage... cpy it		
		msgCp->id = msg->id;
		msgCp->len = msg->len;
		memcpy(msgCp->data,msg->data,msg->len);
		// Put the newly allocate data to queue
		queue.put(msgCp);
	}
}

// ==================== SLAVE THINGS ====================

CANCommunicatorSlave::CANCommunicatorSlave(CANInterface *can, uint8_t bus, CANDiagCalculator *c):
	CANCommunicatorBase(can,bus,c) {
} 

void CANCommunicatorSlave::Start() {
	DBG("CommunicatorSlave %08x [%d]",calc->GetAddrDst(),bus);
	canItf->AddCallBackForId(bus, calc->GetAddrDst(), this);
}

void CANCommunicatorSlave::Stop() {
	canItf->DelCallBackForId(bus, calc->GetAddrDst());
}

