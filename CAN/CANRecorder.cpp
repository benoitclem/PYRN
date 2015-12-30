
#include "CANRecorder.h"
#include "CANCommon.h"
#include "CAN.h"

#include "MyCallBackIds.h"

#include "MyMemoryAllocator.h"

#include "us_ticker_api.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "CANRecorder.cpp"
#endif
#include "MyDebug.h"

extern MyMemoryAllocator memAlloc;

#define SHIFTS_TO_DIVIDE        5
#define CAN_FRAMES_TO_WAIT      32 // Here be cautious to put a ^2 value bcause we use a >> instead of a / and adapt SHIFTS_TO_DIVIDE
#define NET_FRAME_ALLOCSZ       64
#define NET_FRAME_HEADER_SIZE   8

CANRecorder::CANRecorder(CANInterface *itf, CANRecorderCalculator *calculator): MySensorBase(SENSOR_TYPE_CAN_REC) {
    canItf = itf;
    calc = calculator;
    activated = true;
    // Init learnign stuffs to start learning when the first frame comes
    lStruct.isLearning = true;
    lStruct.nTicks = CAN_FRAMES_TO_WAIT;
    lStruct.tmp = (int*) memAlloc.malloc(sizeof(int)*CAN_FRAMES_TO_WAIT);
    lStruct.oldTick = 0;
    // Allocation data forresults
    dataResult = (char*) memAlloc.malloc(sizeof(char)*NET_FRAME_ALLOCSZ);
    offset = NET_FRAME_HEADER_SIZE + 1 + calc->GetNChunks()*2;

    // Set limits
    nPoints = 0;
    nPointsMax = 1; // Becarefull if you increase the max point the Alloc size is going to be bad

    // Network Frame Structure is id(4) | dt (2) | n (2) | nChunk | entries (n*2) | data (n)
    int nChunks = calc->GetNChunks();

    memset(dataResult,0x0, 8 + nChunks*2 + 1);
    *((uint32_t *)dataResult) = calc->GetRxAddr(); // Apply the RX Address to frame header
    dt = (uint16_t*) (dataResult+4);                 // Take the pointer address to frame header dt
    nSamples = (uint16_t*) (dataResult+6); 
    *((uint8_t*)dataResult+8) = nChunks;
    for(uint8_t i = 0; i<nChunks; i++) {
        // Not super optimized here, we could retrieve the pointer at startup
        CANRecorderData *entry = calc->GetRecordEntry(i);
        *((uint8_t*)dataResult+9+(i*2)) = entry->start;
        *((uint8_t*)dataResult+10+(i*2)) = entry->len;
    }

    DBG_MEMDUMP("header",dataResult,offset);
    //*((uint16_t *)dataResult+6) = 0;               // Put 0 to n
    // set the filters and wire the interface to send the packets
    SetupCalcFilter();
    canItf->AddCallBackForId(CAN_BUS_DONT_CARE, CAN_ID_PROMISCUOUS_MODE, this);
}

CANRecorder::~CANRecorder() {
    memAlloc.free(dataResult); 
}

void CANRecorder::SetupCalcFilter() {
    int bus = canItf->BusFromPins(calc->GetPinH(),calc->GetPinL());
    //canItf->SetFilter(bus,calc->GetRxAddr());
}

void CANRecorder::Capture(char *data, uint16_t *len) {
    // TODO: Need to be mutex protected HERE
    BuffMtx.lock();
    if(*len > offset) {
        //DBG("offset = %02x",offset);
        //*((uint16_t *)(data+6)) = offset;
        *nSamples = offset;
        memcpy(data,dataResult,offset);
        *len = offset;
        nPoints = 0;
    }
    // Reset the stuffs in any case
    offset = NET_FRAME_HEADER_SIZE + 1 + calc->GetNChunks()*2;
    BuffMtx.unlock();
}

void CANRecorder::event(int ID, void *data) {
    if(((ID&0x0f) == CAN_CALLBACK) && activated ){
        uint8_t bus = (ID>>8) & 0x0f;
        // Remember we don't own the CANMessage here.
        CANMessage *msg = (CANMessage*) data; 
        int id = msg->id;
        char *data = (char*)msg->data;
        
        if(id == calc->GetRxAddr()) {
            //DBG("RX: %d %d",id,calc->GetRxAddr());
            if(lStruct.isLearning) {
                if(lStruct.nTicks) {
                    if(lStruct.oldTick == 0) {
                        lStruct.oldTick = us_ticker_read();
                    } else {
                        uint32_t t = us_ticker_read();
                        lStruct.tmp[lStruct.nTicks-1] = lStruct.oldTick - t;
                        lStruct.oldTick = t;
                        lStruct.nTicks--;
                    }
                } else {
                    uint32_t sum = 0; 
                    for(uint8_t i = 0; i<CAN_FRAMES_TO_WAIT; i++) {
                        sum += lStruct.tmp[i];
                    }
                    memAlloc.free(lStruct.tmp); 
                    *dt = sum >> 5;
                    DBG("computed dt = %d %04x",*dt,*dt);
                    lStruct.isLearning = false;
                }
            } else {
                if(nPoints < nPointsMax) {
                int n = calc->GetNChunks();
                    for(uint8_t i = 0; i<n; i++) {
                        // Not super optimized here, we could retrieve the pointer at startup
                        CANRecorderData *entry = calc->GetRecordEntry(i);
                        if((offset + entry->len) < NET_FRAME_ALLOCSZ){
                            BuffMtx.lock();
                            //DBG("adding %d current size is %d", entry->len, offset);
                            memcpy(dataResult+offset,data+entry->start,entry->len);
                            offset += entry->len;
                            BuffMtx.unlock();
                        } else {
                            DBG("full");
                        }
                    }
                    nPoints += 1;
                } /*else {
                    DBG("Already acquiered");
                }*/
            }
        }
    }
}
