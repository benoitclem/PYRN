

#include "CANInterface.h"

#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "CANInterface.cpp"
#endif
#include "MyDebug.h"
#include "CANCommon.h"
#include "CANInterface.h"
#include "MyCallBackIds.h"

#define CAN_THREAD_STACK_SIZE   768

static CAN can1(p9, p10);
static CAN can2(p30, p29);

DigitalOut led3(LED3);
DigitalOut led4(LED4);

/*
void cbRx1(void) {
    CANMessage m;
    can1.read(m);
    if(led3 == 1)
        led3 = 0;
    else
        led3 = 1;
}

void cbRx2(void) {
    CANMessage m;
    can2.read(m);
    if(led4 == 1)
        led4 = 0;
    else
        led4 = 1;
}
*/

CANInterface::CANInterface(int CanSpeed1, uint8_t CPH1, uint8_t CPL1, int CanSpeed2,  uint8_t CPH2, uint8_t CPL2):
    MyThread("CANInterface",CAN_THREAD_STACK_SIZE) {
    nCurrentCbs = 0;
    
    // Setup the physical interfaces
    bus1 = &can1;
    bus1->reset();
    //bus1->attach(&cbRx1);
    bus1->attach(this,&CANInterface::RxCbCan1,CAN::RxIrq);
	bus1->frequency(CanSpeed1);
	CanPinH1 = CPH1;
	CanPinL1 = CPL1;
	
    bus2 = &can2;
    bus2->reset();
    //bus2->attach(&cbRx2); //,CAN::RxIrq);
    bus2->attach(this,&CANInterface::RxCbCan2,CAN::RxIrq);
	bus2->frequency(CanSpeed2);
	CanPinH2 = CPH2;
	CanPinL2 = CPL2;
	
    led3 = 1;
    led4 = 1;
}

void CANInterface::Configure(uint8_t bus, int speed) {
    DBG("Setting itf(%d) to %d", bus,speed);
    CAN* b = (bus == 1)? bus1:bus2;
    b->frequency(speed);
}

void CANInterface::AddCallBackForId(uint8_t bus, uint32_t id, MyCallBack *cb) {
    // TODO: Protect this with a mutex
    if(idCbTableMutex.lock(1000) != osEventTimeout) {
        if(nCurrentCbs == CAN_SIMULTANEOUS_CBS) {
            WARN("Sorry Dude, this is not gonna work, too much callback are recorded");
        } else {
            DBG("Adding Entry [%08x(can%d)] - %p",id,bus,cb);
            idCbEntry *newEntry = idCbTable+nCurrentCbs;
			newEntry->bus = bus;
            newEntry->id = id;
            newEntry->cb = cb;
            nCurrentCbs++;
        }
        idCbTableMutex.unlock();
    } else
        ERR("Could not capture the cbtable mutex!");
}
    
void CANInterface::DelCallBackForId(uint8_t bus, uint32_t id) {
    // TODO: Protect this with a mutex
    if(idCbTableMutex.lock(1000) != osEventTimeout) {
        if(nCurrentCbs) {
            for(int i = 0; i<nCurrentCbs; i++){
                if((idCbTable[i].bus == bus) && (idCbTable[i].id == id)) {
                    DBG("Removing Entry [%08x]",id);
                    memcpy(idCbTable+i,idCbTable+(i+1),nCurrentCbs-(i+1));
                    nCurrentCbs--;
                    break;
                }
            }
        }
        idCbTableMutex.unlock();
    } else
        ERR("Could not capture the cbtable mutex!");
}

void CANInterface::DispatchCANMessage(uint8_t bus, CANMessage *msg) {
    for(int i = 0; i<nCurrentCbs; i++){
	    // Check if the entry is promiscuous
	    if(idCbTable[i].id == CAN_ID_PROMISCUOUS_MODE)
	        idCbTable[i].cb->event(CAN_CALLBACK|((bus)<<8),(void*) msg);
		// Check we the cb is listening on the right bus and id number 
		if((idCbTable[i].bus == bus) && (idCbTable[i].id == msg->id)) {
	        // Becarefull with the pointer passed (/!\ make a cpy in the cb)
	        idCbTable[i].cb->event(CAN_CALLBACK|((bus)<<8),(void*) msg);
	        break;    
		}
    }
}

int CANInterface::Send(uint8_t bus, uint32_t id, char *data, uint8_t len) {
    if(sendMutex.lock(1000) != osEventTimeout) {
		CANPrintWorkAround("CANItf TX",bus,id,data,len);
		//DBG("Sending CAN [%08x]-%02X %02X %02X %02X %02X %02X %02X %02X",id,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
        int ret = 0;
        // No Padding but could be implemented here or in the upper layers
        msgTx.id = id;
		msgTx.len = len;
        memcpy(msgTx.data,data,len);
        // Need to check who is responsible for CANMessage memory management
        if(bus == 1) {
            ret = bus1->write(msgTx);
        } else if(bus == 2) {
            ret = bus2->write(msgTx);
		} else {
 			DBG("Send: bus %d does not exist",bus);
		}
		sendMutex.unlock();
        return ret;
    } else
        ERR("Could not capture the sending mutex!");

    return 0;
}

int CANInterface::Receive(uint32_t id, char *datan, uint8_t *len) {
	// Not sure of how this could work like that
	return 0;
}

// WARN: If we got issue of stack ovf we can reduce function call 
void CANInterface::ProcessMsgFromRxFifo(uint8_t bus) {
    bool gotMsg = false;
	// ========= Stuffs for bus 1 =========
	CANMessage msg;
	// == This section need to be atomic ==
	NVIC_DisableIRQ(CAN_IRQn);
	if(bus == 1) {
		gotMsg = fifo1RxMsg.get(&msg);
	} else if(bus == 2) {
		gotMsg = fifo2RxMsg.get(&msg);
	} else {
		DBG("ProcessMsg: bus %d does not exist",bus);
	}
	NVIC_EnableIRQ(CAN_IRQn);
	// ====================================
	if(gotMsg)
		DispatchCANMessage(bus,&msg);
}

void CANInterface::Main(void) {
    int i = 0;
    // TODO: Need to be sure that the irq is not firing multiple time before reading
    while(running){
        //if((i%100) == 1)
        //    DBG("CanInterface Running");
        if((bus1->rderror() != 0) || (bus2->rderror() != 0)) {
            ERR("Got Hardware rx errors ovf, need reset?");
           // bus1->reset();
           // bus2->reset();
        }
        if( rxOvf ) {
            ERR("Got Software rx errors ovf, need to extend FIFO!!");
            rxOvf = false;
        }
		ProcessMsgFromRxFifo(1);
		ProcessMsgFromRxFifo(2);
        Thread::wait(1);
        i++;
    }
}

uint8_t CANInterface::BusFromPins(uint8_t CanH, uint8_t CanL) {
	if((CanL == CanPinL1) && (CanH == CanPinH1)) {
		return 1;
	}
	if((CanL == CanPinL2) && (CanH == CanPinH2)) {
		return 2;
	}
	return 0;
}

void CANInterface::event(int ID, void *data) {

}

void CANInterface::RxCbCan1(void) {
    __disable_irq();
    CANMessage m;
    if(bus1->read(m)) {
        rxOvf = !fifo1RxMsg.put(m);
        if(led3 == 1){
            led3 = 0;
        } else {
            led3 = 1;
        }
    }
    __enable_irq();
}

void CANInterface::RxCbCan2(void) {
    __disable_irq();
    CANMessage m;
    if(bus2->read(m)) {
        rxOvf = !fifo2RxMsg.put(m);
        if(led4 == 1) {
            led4 = 0;
        } else {
            led4 = 1;
        }
    }
    __enable_irq();
}
