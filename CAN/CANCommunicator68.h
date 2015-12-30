
#ifndef CAN_COMMUNICATOR_68_H
#define CAN_COMMUNICATOR_68_H

#include "CANCommunicators.h"

class CANCommunicator68: public CANCommunicatorMaster {
public:
	CANCommunicator68(CANInterface *can, uint8_t bus, CANDiagCalculator *c);
	virtual void ExecuteCommand(char *data, uint8_t len, char *response = NULL, uint8_t *respLen = NULL, uint32_t msTmo = 0);
};

#endif // CAN_COMUNICATOR_68
