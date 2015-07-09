

#ifndef CAN_DIAG_SENSOR_H
#define CAN_DIAG_SENSOR_H

#include "CANDiagCalculator.h"

#include "MySensor.h"
#include "MyCallBack.h"
#include "CANInterface.h"
#include "CANCommunicator6A.h"
#include "MyMemoryAllocator.h"

#define CAN_DIAG_STORE_SIZE		128
#define CAN_DIAG_THREAD_STACK_SIZE   2*1280

class CANDiagSensorBase: public MySensor, public MyMemoryObject  {
protected:
	RtosTimer			KeepAliveTimer;
	char 				store[CAN_DIAG_STORE_SIZE];
	unsigned char		stack[CAN_DIAG_THREAD_STACK_SIZE];
	CANDiagCalculator 	*calculator;
public:
    CANDiagSensorBase(CANDiagCalculator *calc);
	// Where we prepare the result storage
	// Each Diag Type know the kind of result it will have
	virtual void InitResultsStatic();
	// We start the rtos timer here
	virtual void KeepAliveStart();
	// We stop the rtos timer here
	virtual void KeepAliveStop();
	// Sequence that send the keep alive (called by rtos timer cb)
	virtual void KeepAliveSequence() = 0;
	// Init Diag Sequence
	virtual void DiagStart() = 0;
	// Stop Diag Sequence
	virtual void DiagStop() = 0;
	// The Diag Sequence, where we send the cmd
	virtual void DiagSequence() = 0;
	// Where we take the CAN Fashion Results and put them into stringified form
	virtual void StoreLastImpact(void) = 0;
	// The Hard work is done here
	virtual void Loop();
	// Get Internal Pointers
	virtual CANDiagCalculator *GetDiagCalculator(void) { return calculator; };
};

class CANDiagSensor6A: public CANDiagSensorBase {
protected:
	CANCommunicator6A *ComHandler;
	// The maintain is sended as a cmd so 
	Mutex DiagTxCMD;
public:
	CANDiagSensor6A(CANDiagCalculator *calc, CANCommunicator6A *com);
	virtual void KeepAliveSequence();
	virtual void DiagStart();
	virtual void DiagStop() {};
	virtual void DiagSequence();
	virtual void StoreLastImpact(void);
	virtual CANCommunicator6A *GetCommunicator(void) { return ComHandler; };
};

#endif // CAN_DIAG_SENSOR_H
