

#include "CANDiagSensor.h"
#include "Configs.h"

#define __DEBUG__ CAN_DIAG_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANDiagSensorBase.cpp"
#endif
#include "MyDebug.h"

// =========== The Base Diagnosis ===========

int CalcIndex;

void KeepAliveTimerTrampoline(void const *args){
    CANDiagSensorBase *ds = (CANDiagSensorBase*) args;
	ds->KeepAliveSequence(CalcIndex);
}

CANDiagSensorBase::CANDiagSensorBase(CANDiagCalculator *calc): MySensor(SENSOR_NAME_CAN_DIAG,SENSOR_TYPE_CAN_DIAG,1000,CAN_DIAG_THREAD_STACK_SIZE,stack),
	KeepAliveTimer(&KeepAliveTimerTrampoline,osTimerPeriodic,this) {
	calculators[0] = calc;
	nCalculators  = 1;
	InitResults(SENSOR_RESSZ_DEFAULT);
}

void CANDiagSensorBase::FreeCalculator(int i) {
	if(i<nCalculators){
		if(calculators[i] != NULL) {
			free(calculators[i]);
			calculators[i] = NULL;
			nCalculators -= 1;
		}
	}
}

void CANDiagSensorBase::AddCalculator(CANDiagCalculator *calc, int i) {
	FreeCalculator(i);
	calculators[i] = calc;
	nCalculators += 1;
}

void CANDiagSensorBase::InitResultsStatic() {
    DBG("InitResultStatic have been defined");
    results.start = (uint8_t*)store;
    results.current = (uint8_t*)store;
    results.max = sizeof(store); //CAN_DIAG_STORE_SIZE*sizeof(uint8_t);
}

// We start the rtos timer here
void CANDiagSensorBase::KeepAliveStart(int i) {
	CANDiagCommandHeader *hdrCmd = NULL;
	calculators[i]->FirstCommand(&hdrCmd,CMD_KEEP_ALIVE);
	uint16_t holdingFrequency = *((uint16_t*)(hdrCmd->cmd+(hdrCmd->size-2)));
	DBG("StartKeepAlive (%d)",holdingFrequency);
	if(calculators[i]) {
		CalcIndex = i;
		KeepAliveTimer.start(holdingFrequency);
	}
	/*else
		KeepAliveTimer.stop();*/
}

// We stop the rtos timer here
void CANDiagSensorBase::KeepAliveStop() {
	DBG("StopKeepAlive");
	KeepAliveTimer.stop();
}

void CANDiagSensorBase::Loop() {
	for(int i = 0; i < nCalculators; i++) {
		DBG("LoopStart %d (%p)",i,this);
		DiagStart(i);
		KeepAliveStart(i);
		DiagSequence(i);
		KeepAliveStop();
		DiagStop(i);
		DBG("LoopEnd %d (%p)",i,this);
	}
}

// =========== Specific implementation (6A) ===========
CANDiagSensor6A::CANDiagSensor6A(CANDiagCalculator *calc, CANCommunicator6A *com): CANDiagSensorBase(calc) {
	ComHandler = com;
}

void CANDiagSensor6A::KeepAliveSequence(int i) {
	DBG("[%08x](6A) KeepAliveSequenceStart", calculators[i]->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[32];
	uint8_t len = 0; 
	calculators[i]->FirstCommand(&hdrCmd,CMD_KEEP_ALIVE);
	if(hdrCmd){
		DiagTxCMD.lock();
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size-2,response,&len,100);
		DiagTxCMD.unlock();
	} else {
		ERR("[%08x](6A) Could not find CMD_KEEP_ALIVE to play", calculators[i]->GetCalcId());
	}
	DBG("[%08x](6A) KeepAliveSequenceEnd", calculators[i]->GetCalcId());
}

void CANDiagSensor6A::DiagStart(int i) {
	DBG("[%08x] DiagStartSequence", calculators[i]->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[32];
	uint8_t len = 0; 
	calculators[i]->FirstCommand(&hdrCmd,CMD_DIAG_START);
	if(hdrCmd){
		DiagTxCMD.lock();
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size,response,&len,100);
		DiagTxCMD.unlock();
	} else {
		ERR("[%08x](6A) Could not find CMD_DIAG_START to play", calculators[i]->GetCalcId());
	}
};

void CANDiagSensor6A::DiagSequence(int i) {
	DBG("[%08x](6A) DiagSequence START", calculators[i]->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[64];
	uint8_t len = 0;

	// Get first CMD_NORM to Execute
	calculators[i]->FirstCommand(&hdrCmd,CMD_NORM);
	if(hdrCmd == NULL) {
		ERR("[%08x](6A) Could not find CMD_NORM to play", calculators[i]->GetCalcId());
		return;
	}
	while((hdrCmd != NULL) && running) {
		DBG("[%08x](6A) DiagSequence LOOP", calculators[i]->GetCalcId());
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
			sprintf(s,"[%08x](6A) DiagSequence Receiced",calculators[i]->GetCalcId());
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
				DBG("[%08x](6A) DiagSequence: No more space to record data",calculators[i]->GetCalcId());
			}
		}
		// jump to next 
		calculators[i]->NextCommand(&hdrCmd,CMD_NORM);
	}
	DBG("[%08x](6A) DiagSequence END",calculators[i]->GetCalcId());
}

// =========== Specific implementation (68) ===========
CANDiagSensor68::CANDiagSensor68(CANDiagCalculator *calc, CANCommunicator68 *com): CANDiagSensorBase(calc) {
	ComHandler = com;
}

void CANDiagSensor68::DiagStart(int i) {
	DBG("[%08x](68) DiagStartSequence", calculators[i]->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[32];
	uint8_t len = 0; 
	calculators[i]->FirstCommand(&hdrCmd,CMD_DIAG_START);
	if(hdrCmd){
		DiagTxCMD.lock();
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size,response,&len,100);
		DiagTxCMD.unlock();
	} else {
		ERR("[%08x] Could not find CMD_DIAG_START to play", calculators[i]->GetCalcId());
	}
};

void CANDiagSensor68::DiagSequence(int i) {
	DBG("[%08x](68) DiagSequence START", calculators[i]->GetCalcId());
	CANDiagCommandHeader *hdrCmd = NULL;
	char response[64];
	uint8_t len = 0;

	// Get first CMD_NORM to Execute
	calculators[i]->FirstCommand(&hdrCmd,CMD_NORM);
	if(hdrCmd == NULL) {
		ERR("[%08x](68) Could not find CMD_NORM to play", calculators[i]->GetCalcId());
		return;
	}
	while((hdrCmd != NULL) && running) {
		DBG("[%08x](68) DiagSequence LOOP", calculators[i]->GetCalcId());
		// the CMD 
		//DBG("BEFORE THE LOCK %p",this);
		DiagTxCMD.lock();
		//DBG("AFTER THE LOCK %p",this);
		ComHandler->ExecuteCommand(hdrCmd->cmd,hdrCmd->size,response,&len, 100);
		DiagTxCMD.unlock();
		// the result is recorded
		//if(false){
		if(len){
			char s[40];
			sprintf(s,"[%08x](68) DiagSequence Receiced",calculators[i]->GetCalcId());
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
				DBG("[%08x](68) DiagSequence: No more space to record data",calculators[i]->GetCalcId());
			}
		}
		// jump to next 
		calculators[i]->NextCommand(&hdrCmd,CMD_NORM);
	}
	DBG("[%08x](68) DiagSequence END",calculators[i]->GetCalcId());
}
