
#ifndef CAN_COMMON_H
#define CAN_COMMON_H

#include "mbed.h"

// The workaroud seems to reduc e stack usage

extern "C" {
	void CANPrintWorkAround(const char* msg, uint8_t bus, uint32_t id, char *data, uint8_t len);
}

#endif // CAN_COMMON_H
