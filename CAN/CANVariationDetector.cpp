

#include "CANVariationDetector.h"

#include "CAN.h"
#include "CANFifoMessage.h"
#include "CANCommon.h"
#include "MyCallBackIds.h"
#include "Configs.h"

#define __DEBUG__ CAN_VARIATION_DETECTOR_DBG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANVariationDetector.cpp"
#endif
#include "MyDebug.h"


CANVariationDetector::CANVariationDetector(CANInterface *itf, int bus) {
	canItf = itf;
	nFrameEntry = 0;
	if(bus == 0)
		itf->AddCallBackForId(CAN_BUS_DONT_CARE, CAN_ID_PROMISCUOUS_MODE, this);
	else
		itf->AddCallBackForId(bus,CAN_ID_PROMISCUOUS_MODE,this);
}


void CANVariationDetector::event(int ID, void *data) {
    //char buff[23];
    if((ID&0x0f) == CAN_CALLBACK) {
		uint8_t bus = (ID>>8) & 0x0f;
        // Remember we don't own the CANMessage here.
        CANMessage *msg = (CANMessage*) data;
        int id = msg->id;
        int len = msg->len;
        char *canData = (char*)msg->data;
        //CANPrintWorkAround("Variation RX",bus,id,canData,len);
        // Look if the frame have been registered yet
        for(int i = 0; i<nFrameEntry; i++){
        	if(fe[i].id == id) {
        		for(int j = 0; j < len; j++){
        			if(fe[i].data[j] != canData[j]){
        				// Tell python the frame data did change
        				DBG("%08x %d %d", id, j, canData[j]);
        				// Replace old data 
        				// We could do the copy in one chunk after comparison)
        				fe[i].data[j] = canData[j];
        			}
        		}
        		// All the incoming data been processed go out
        		return;
        	}
        } 
        // The id is not inserted for now, insert it
        DBG("Insert new id = %08x",id);
        fe[nFrameEntry].id = id;
        memcpy(fe[nFrameEntry].data,canData,len);
        nFrameEntry++; 
    }
}