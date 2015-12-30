#ifndef CAN_RECORDER_CALCULATOR_H
#define CAN_RECORDER_CALCULATOR_H

#include "mbed.h"
#include "MyMemoryAllocator.h"

#define CAN_RECORDER_ENTRY_STORAGE 	8
#define CAN_RECORDER_HDR_TYPE 		2

typedef struct _CANRecorderCalculatorHeader {
	uint8_t		hdrType;
	uint32_t	idCalc;
	uint16_t 	speed;
	uint32_t	rxAddr;
	uint8_t		pinh;
	uint8_t		pinl;
	uint8_t		nChunks;
	char 		data[];
} __attribute__((packed)) CANRecorderCalculatorHeader;

typedef struct _CANRecorderData {
	uint8_t 	start;
	uint8_t		len;
} __attribute__((packed)) CANRecorderData;

class CANRecorderCalculator: public MyMemoryObject {
protected:
	bool dataPresence;
	bool dataValidity;
	CANRecorderCalculatorHeader calc;
	CANRecorderData *recordEntries;
	char recordData[CAN_RECORDER_ENTRY_STORAGE*sizeof(CANRecorderData)];
public:
	CANRecorderCalculator(const char *buff = NULL);
	int SetData(const char *buff);
	char* GetDataPointer() {return (char*)&calc;};
	int Parse(const char *buff);
	bool Validate(void);
	bool Ready(void);
	CANRecorderData *GetRecordEntry(uint8_t index);
	uint32_t GetCalcId() {return calc.idCalc;};
	uint16_t GetSpeed() {return calc.speed;};
	uint32_t GetRxAddr() {return calc.rxAddr;};
	uint8_t  GetPinH() {return calc.pinh;};
	uint8_t  GetPinL() {return calc.pinl;};
	uint8_t  GetNChunks() {return calc.nChunks;};
	char *   GetChunks() {return calc.data;};
};

#endif