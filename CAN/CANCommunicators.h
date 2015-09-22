
#ifndef CAN_COMMUNICATORS
#define CAN_COMMUNICAT

#include "rtos.h"
#include "MyCallBack.h"
#include "CANInterface.h"
#include "CANDiagCalculator.h"
#include "MyMemoryAllocator.h"

/*
 * The communicator Base keep the base information
 */

class CANCommunicatorBase: public MyCallBack, public MyMemoryObject {
protected:
	// Used as RX or TX buffers
	unsigned char buff[64];
	uint16_t bufflen;

	CANInterface *canItf;
	uint8_t bus;			// The bus id to talk with
	// uint32_t idSrc;		// The can id to use when talking
	// uint32_t idDest;	 	// The can id to listen
	CANDiagCalculator *calc;
public:
	CANCommunicatorBase(CANInterface *can, uint8_t b, CANDiagCalculator *c);
	virtual void Configure(uint8_t bus, CANDiagCalculator *c);
	// CallBack 
    virtual void event(int ID, void *data) = 0;
};

/* The communicator Master know how to request a calculator:
 * 		how to package a command,
 * 		the kind of reply a calculator can give,
 * 		how to handle the ack when needed,
 * 		... 
 */

class CANCommunicatorMaster: public CANCommunicatorBase {
protected:
	Queue<CANMessage,10> queue;
public:
	CANCommunicatorMaster(CANInterface *can, uint8_t bus, CANDiagCalculator *c);
	virtual void FlushQueue(void);
	virtual void ExecuteCommand(char *data, uint8_t len, char *response = NULL, uint8_t *respLen = NULL, uint32_t msTmo = 1000) = 0;
	// CallBack 
    virtual void event(int ID, void *data);
};

/* The communicator Slave know how to respond to a master:
 * 		the kind of response the master waits
 */

class CANCommunicatorSlave: public CANCommunicatorBase {
public:
	typedef struct _cmd {
		uint8_t		bus;
		uint32_t	id;
		char 		msg[8];
		uint8_t		len;
	} cmd ;
	CANCommunicatorSlave(CANInterface *can, uint8_t bus,  CANDiagCalculator *c);
	void Start();
	void Stop();
	virtual uint8_t RequestDB(char *cmd, uint8_t clen, char *results, uint8_t *rlen) = 0;
    virtual void event(int ID, void *data) = 0;
};

#endif // CAN_COMMUNICATION_HANDLER
