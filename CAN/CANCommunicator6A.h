
#ifndef CAN_COMMUNICATOR_6A_H
#define CAN_COMMUNICATOR_6A_H

#include "CANCommunicators.h"

class CANCommunicator6A: public CANCommunicatorMaster {
protected:
	enum _state {
		IDLE,		
		EXT_CMD,
	} state;
public:
	CANCommunicator6A(CANInterface *can, uint8_t bus, CANDiagCalculator *c);
	virtual void ExecuteCommand(char *data, uint8_t len, char *response = NULL, uint8_t *respLen = NULL, uint32_t msTmo = 0);
};

class CANCommunicatorSim6A: public CANCommunicatorSlave {
protected:
	enum _state {
		IDLE,		
		EXT_CMD,
	} state;
	bool inDiag;
	char iteration;
	char response[32];
	uint8_t rLen;
public:
	CANCommunicatorSim6A(CANInterface *can, uint8_t bus, CANDiagCalculator *c);
	virtual uint8_t RequestDB(char *cmd, uint8_t clen, char *results, uint8_t *rlen);
	virtual void FormatResponse(char *response, uint8_t responseSz, char *payload, uint8_t *payloadSz, uint8_t it);
    virtual void event(int ID, void *data);
};

#endif // CAN_COMUNICATOR_6A_H
