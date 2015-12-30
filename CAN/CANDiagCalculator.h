
#ifndef CAN_DIAG_CALCULATOR_H
#define CAN_DIAG_CALCULATOR_H

#include "mbed.h"
#include "MyMemoryAllocator.h"

#define CMD_END			0
#define CMD_ACK			1
#define CMD_KEEP_ALIVE	2
#define CMD_NORM		3
#define CMD_DIAG_START	4

#define CAN_DIAG_CMD_STORAGE 256
#define CAN_DIAG_HDR_TYPE 		1

typedef struct _CANDiagCalculatorHeader {
	uint8_t					hdrType;
	uint32_t				idCalc;
	uint8_t 				diagCode;
	uint16_t 				speed;
	//uint32_t				initAddrSrc;
	//uint32_t				initAddrDst;
	uint32_t				addrDst;
	uint32_t				addrSrc;
	uint8_t					pinh;
	uint8_t					pinl;
	uint16_t				cmdLen;
}__attribute__((packed)) CANDiagCalculatorHeader;

typedef struct _CANDiagCommandHeader {
	uint16_t	type;
	uint32_t	idCmd;
	uint8_t 	size;
	char 		cmd[];
}__attribute__((packed)) CANDiagCommandHeader;

class CANDiagCalculator: public MyMemoryObject {
protected:
	bool dataPresence;
	bool dataValidity;
	CANDiagCalculatorHeader calc;
	CANDiagCommandHeader *cmd;
	char cmdStorage[CAN_DIAG_CMD_STORAGE];
public:
	CANDiagCalculator(const char *buff = NULL);
	int SetData(const char *buff);
	int Parse(const char *buff);
	bool Validate(void);
	bool Ready(void);
	void FirstCommand(CANDiagCommandHeader **pCmd, uint16_t cmdType);
	void NextCommand(CANDiagCommandHeader **pCmd, uint16_t cmdType);
	char* GetDataPointer() {return (char*)&calc;};
	uint32_t GetCalcId() {return calc.idCalc;};
	uint8_t  GetDiagCode() {return calc.diagCode;};
	uint16_t GetSpeed() {return calc.speed;};
	uint32_t GetAddrSrc() {return calc.addrSrc;};
	uint32_t GetAddrDst() {return calc.addrDst;};
	uint8_t  GetPinH() {return calc.pinh;};
	uint8_t  GetPinL() {return calc.pinl;};
	uint8_t  GetCmdLen() {return calc.cmdLen;};
};

#endif // CAN_DIAG_SENSOR_H
