

#include "CANDiagSensor.h"
#include "Configs.h"

#define __DEBUG__ CAN_DIAG_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANDiagSensorBase.cpp"
#endif
#include "MyDebug.h"

// =========== The Base Diagnosis ===========

void KeepAliveTimerTrampoline(void const *args){
    CANDiagSensorBase *ds = (CANDiagSensorBase*) args;
	ds->KeepAliveSequence();
}

CANDiagSensorBase::CANDiagSensorBase(CANDiagCalculator *calc): MySensor(SENSOR_NAME_CAN_DIAG,SENSOR_TYPE_CAN_DIAG,1000,CAN_DIAG_THREAD_STACK_SIZE,stack),
	KeepAliveTimer(&KeepAliveTimerTrampoline,osTimerPeriodic,this) {
	calculator = calc;
	InitResults(SENSOR_RESSZ_DEFAULT);
}

void CANDiagSensorBase::InitResultsStatic() {
    DBG("InitResultStatic have been defined");
    results.start = (uint8_t*)store;
    results.current = (uint8_t*)store;
    results.max = sizeof(store); //CAN_DIAG_STORE_SIZE*sizeof(uint8_t);
}

// We start the rtos timer here
void CANDiagSensorBase::KeepAliveStart() {
	CANDiagCommandHeader *hdrCmd = NULL;
	calculator->FirstCommand(&hdrCmd,CMD_KEEP_ALIVE);
	uint16_t holdingFrequency = *((uint16_t*)(hdrCmd->cmd+(hdrCmd->size-2)));
	DBG("StartKeepAlive (%d)",holdingFrequency);
	if(calculator)
		KeepAliveTimer.start(holdingFrequency);
	/*else
		KeepAliveTimer.stop();*/
}

// We stop the rtos timer here
void CANDiagSensorBase::KeepAliveStop() {
	DBG("StopKeepAlive");
	KeepAliveTimer.stop();
}

void CANDiagSensorBase::Loop() {
	DBG("LoopStart (%p)",this);
	DiagStart();
	KeepAliveStart();
	DiagSequence();
	KeepAliveStop();
	DiagStop();
	DBG("LoopEnd (%p)",this);
}

// =========== Specific implementation ===========
CANDiagSensor6A::CANDiagSensor6A(CANDiagCalculator *calc, CANCommunicator6A *com): CANDiagSensorBase(calc) {
	ComHandler = com;
}

void CANDiagSensor6A::KeepAliveSequence() {
	DBG("[%08x] KeepAliveSequenceStart", calculator->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[32];
	uint8_t len = 0; 
	calculator->FirstCommand(&hdrCmd,CMD_KEEP_ALIVE);
	if(hdrCmd){
		DiagTxCMD.lock();
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size-2,response,&len,100);
		DiagTxCMD.unlock();
	} else {
		ERR("[%08x] Could not find CMD_KEEP_ALIVE to play", calculator->GetCalcId());
	}
	DBG("[%08x] KeepAliveSequenceEnd", calculator->GetCalcId());
}

void CANDiagSensor6A::DiagStart() {
	DBG("[%08x] DiagStartSequence", calculator->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[32];
	uint8_t len = 0; 
	calculator->FirstCommand(&hdrCmd,CMD_DIAG_START);
	if(hdrCmd){
		DiagTxCMD.lock();
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size,response,&len,100);
		DiagTxCMD.unlock();
	} else {
		ERR("[%08x] Could not find CMD_DIAG_START to play", calculator->GetCalcId());
	}
};

void CANDiagSensor6A::DiagSequence(){
	DBG("[%08x] DiagSequence START", calculator->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[64];
	uint8_t len = 0;

	// Get first CMD_NORM to Execute
	calculator->FirstCommand(&hdrCmd,CMD_NORM);
	if(hdrCmd == NULL) {
		ERR("[%08x] Could not find CMD_NORM to play", calculator->GetCalcId());
		return;
	}
	while(hdrCmd && running) {
		DBG("[%08x] DiagSequence LOOP", calculator->GetCalcId());
		// the CMD 
		//DBG("BEFORE THE LOCK %p",this);
		DiagTxCMD.lock();
		//DBG("AFTER THE LOCK %p",this);
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size,response,&len, 100);
		DiagTxCMD.unlock();
		// the result is recorded
		//if(false){
		if(len){
			char s[32];
			sprintf(s,"[%08x] DiagSequence Receiced",calculator->GetCalcId());
			DBG_MEMDUMP((const char*)s,response,len);
			if((results.num+4+2+len)<results.max){
				if(resultsMutex.trylock()) {
					// Store the Cmd Index
					*((uint32_t*)results.current) = hdrCmd->idCmd;
					results.current+=4;
					// Store the Cmd Index
					*((uint32_t*)results.current) = time(NULL);
					results.current+=4;
					// Store the Cmd Response Len
					*((uint16_t*)results.current) = len;
					results.current += 2;
					// Store the results
					memcpy((char*)results.current,response,len);
		            results.current += len;
		            results.num += 4+4+2+len; // header + data
					resultsMutex.unlock();
				}
			} else {
				DBG("[%08x] DiagSequence: No more space to record data",calculator->GetCalcId());
			}
		}
		// jump to next 
		calculator->NextCommand(&hdrCmd,CMD_NORM);
	}
	DBG("[%08x] DiagSequence END",calculator->GetCalcId());
}

// Not use in diag. StoreLastImpact is a post-action recording
// Here we use a live recording, each time we got a result in Diag Sequence
// we need to record it. So here, this is just and empty method
void CANDiagSensor6A::StoreLastImpact(void) {
	//DBG("STORE IMPACT");
}

