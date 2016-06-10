
#include "CANIdTest.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "CANIdTest.cpp"
#endif
#include "MyDebug.h"

CANIdTest::CANIdTest(CANInterface *itf, int32_t lId){
    if(itf) {
        canItf = itf;	
		itf->AddCallBackForId(CAN_BUS_DONT_CARE, CAN_ID_PROMISCUOUS_MODE, this);
    }
}

void CANIdTest::event(int ID, void *data) {
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
