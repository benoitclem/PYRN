

#ifndef CAN_DIAG_SENSOR_H
#define CAN_DIAG_SENSOR_H

#include "CANDiagCalculator.h"

#include "MySensor.h"
#include "MyCallBack.h"
#include "CANInterface.h"
#include "CANCommunicator6A.h"
#include "CANCommunicator68.h"
#include "MyMemoryAllocator.h"
#include "Configs.h"

class CANDiagSensorBase: public MySensor, public MyMemoryObject  {
protected:
	CANCommunicatorMaster *ComHandler;
	RtosTimer			KeepAliveTimer;
	char 				store[CAN_DIAG_STORE_SIZE];
	unsigned char		stack[CAN_DIAG_THREAD_STACK_SIZE];
	CANDiagCalculator 	*calculators[2];
	int 				nCalculators;
public:
    CANDiagSensorBase(CANDiagCalculator *calc);
    // Allow to free a calculator OTA
    virtual void FreeCalculator(int i) ;
    // Allow to add a new calculator freeing the inplace calculator if exists
	virtual void AddCalculator(CANDiagCalculator *calc, int i);
	// Where we prepare the result storage
	// Each Diag Type know the kind of result it will have
	virtual void InitResultsStatic();
	// We start the rtos timer here
	virtual void KeepAliveStart(int i);
	// We stop the rtos timer here
	virtual void KeepAliveStop();
	// Sequence that send the keep alive (called by rtos timer cb)
	virtual void KeepAliveSequence(int i) = 0;
	// Init Diag Sequence
	virtual void DiagStart(int i) = 0;
	// Stop Diag Sequence
	virtual void DiagStop(int i) = 0;
	// The Diag Sequence, where we send the cmd
	virtual void DiagSequence(int i) = 0;
	// Where we take the CAN Fashion Results and put them into stringified form
	virtual void StoreLastImpact(void) = 0;
	// The Hard work is done here
	virtual void Loop();
	// Get Internal Pointers
	virtual CANDiagCalculator *GetDiagCalculator(int i) { return calculators[i]; };
	// Get the quantity of calculators
	virtual int GetNDiagCalculator(void) { return nCalculators; };
	// Get the CAN communicator Handler
	virtual CANCommunicatorMaster *GetCommunicator(void) { return ComHandler; };
};

class CANDiagSensor6A: public CANDiagSensorBase {
protected:
	// The maintain is sended as a cmd so 
	Mutex DiagTxCMD;
public:
	CANDiagSensor6A(CANDiagCalculator *calc, CANCommunicator6A *com);

	virtual void KeepAliveSequence(int i);

	virtual void DiagStart(int i);
	virtual void DiagStop(int i) {};
	virtual void DiagSequence(int i);

	virtual void StoreLastImpact(void) {};
};

class CANDiagSensor68: public CANDiagSensorBase {
protected:
	// The maintain is sended as a cmd so 
	Mutex DiagTxCMD;
public:
	CANDiagSensor68(CANDiagCalculator *calc, CANCommunicator68 *com);
	// No keep alive in 68 mode
	virtual void KeepAliveStart(int i) {};
	virtual void KeepAliveStop() {};
	virtual void KeepAliveSequence(int i) {};
	
	virtual void DiagStart(int i);
	virtual void DiagStop(int i) {};
	virtual void DiagSequence(int i);

	virtual void StoreLastImpact(void) {};
};

#endif // CAN_DIAG_SENSOR_H
