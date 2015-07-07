
#include "CANSniffer.h"
#include "CANCommon.h"
#include "CAN.h"

#include "MyCallBackIds.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "CANSniffer.cpp"
#endif
#include "MyDebug.h"

CANSniffer::CANSniffer(CANInterface *itf, int32_t *lId, uint16_t len){
    if(itf) {
        canItf = itf;	
		// You can pass a list of Sniffer IDs (his is a way to do a software filter)
		if(lId == NULL)
        	itf->AddCallBackForId(CAN_BUS_DONT_CARE, CAN_ID_PROMISCUOUS_MODE, this);
		else {
			for(uint16_t i = 0; i < len ; i++) {
				itf->AddCallBackForId(CAN_BUS_DONT_CARE, *lId, this);
				lId++;
			}		
		}
    }
}

void CANSniffer::event(int ID, void *data) {
    //char buff[23];
    if((ID&0x0f) == CAN_CALLBACK) {
		uint8_t bus = (ID>>8) & 0x0f;
        // Remember we don't own the CANMessage here.
        CANMessage *msg = (CANMessage*) data; 
        //sprintf(buff,"CANSniffer [%08x]:",msg->id);
        //DBG_MEMDUMP((const char*)buff,(const char*)msg->data,msg->len);
        int id = msg->id;
        char *data = (char*)msg->data;
		CANPrintWorkAround("SNIFFER RX",bus,id,data,msg->len);        
		//DBG("Received CAN [%08x]-%02X %02X %02X %02X %02X %02X %02X %02X",id,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
    }
}
