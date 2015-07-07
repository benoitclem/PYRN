
#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include "MyThread.h"
#include "MyCallBack.h"
#include "CAN.h"
#include "CANFifoMessage.h"

#define CAN_ID_PROMISCUOUS_MODE 0xffffffff      

#define CAN_SIMULTANEOUS_CBS    8
#define CAN_SIMULTANEOUS_MSG    15

#define CAN_BUS_1				1
#define CAN_BUS_2				2
#define CAN_BUS_DONT_CARE		-1

class CANInterface: public MyThread, public MyCallBack {
protected:
    typedef struct _idCbEntry{
		uint8_t		bus;
        uint32_t 	id;            // 29b and 11b can ids are supported
        MyCallBack *cb;
    } idCbEntry;
    
    Mutex idCbTableMutex;
    Mutex sendMutex;
    
    uint8_t nCurrentCbs;           // How much entries we got in our table
    idCbEntry idCbTable[CAN_SIMULTANEOUS_CBS];     // This mean that we allow at most 8 sensors to receive events simultaneously
    
    CAN *bus1;
    CAN *bus2;
 
 	uint8_t CanPinH1;
  	uint8_t CanPinL1;	
 	uint8_t CanPinH2;
 	uint8_t CanPinL2;
 	  
    bool rxOvf;
    CANFifoMessage fifo1RxMsg;
	CANFifoMessage fifo2RxMsg;

    CANMessage msgTx;

public:
    CANInterface(int CanSpeed1 = 500000, uint8_t CanPinH1 = 3, uint8_t CanPinL1 = 8, int CanSpeed2 = 500000,  uint8_t CanPinH2 = 6, uint8_t CanPinL2 = 14);
    virtual void Configure(uint8_t ifaceNumber, int speed);
    virtual void AddCallBackForId(uint8_t bus, uint32_t id, MyCallBack *cb);
    virtual void DelCallBackForId(uint8_t bus, uint32_t id);
    virtual void DispatchCANMessage(uint8_t bus, CANMessage *msg);
    virtual int  Send(uint8_t bus,uint32_t id, char *data, uint8_t len);
	virtual int  Receive(uint32_t id, char *datan, uint8_t *len);
	virtual void ProcessMsgFromRxFifo(uint8_t bus);
    virtual void Main();
    
    virtual uint8_t BusFromPins(uint8_t CanH, uint8_t CanL);

	// Software Callbacks
	virtual void event(int ID, void *data);
    
	// Hardware IRQ
    virtual void RxCbCan1(void);
    virtual void RxCbCan2(void);
   
    virtual CAN *GetCan1() { return bus1; }
    virtual CAN *GetCan2() { return bus2; }
};

#endif
