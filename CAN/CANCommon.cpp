
#include "CANCommon.h"
#include "Configs.h"

#define __DEBUG__ CAN_WORKAROUND_LVL
#ifndef __MODULE__
#define __MODULE__ "CANCommon.cpp"
#endif
#include "MyDebug.h"

extern "C" {

void CANPrintWorkAround(const char* msg, uint8_t bus, uint32_t id, char *data, uint8_t len) {
	switch(len) {
	default:
	case 0:
		break;
	case 1:
		DBG("%s [%d|%08x|%d|%02X]",msg,bus,id,len,data[0]);
		break;
	case 2:
		DBG("%s [%d|%08x|%d|%02X %02X]",msg,bus,id,len,data[0],data[1]);
		break;
	case 3:
		DBG("%s [%d|%08x|%d|%02X %02X %02X]",msg,bus,id,len,data[0],data[1],data[2]);
		break;
	case 4:
		DBG("%s [%d|%08x|%d|%02X %02X %02X %02X]",msg,bus,id,len,data[0],data[1],data[2],data[3]);
		break;
	case 5:
		DBG("%s [%d|%08x|%d|%02X %02X %02X %02X %02X]",msg,bus,id,len,data[0],data[1],data[2],data[3],data[4]);
		break;
	case 6:
		DBG("%s [%d|%08x|%d|%02X %02X %02X %02X %02X %02X]",msg,bus,id,len,data[0],data[1],data[2],data[3],data[4],data[5]);
		break;
	case 7:
		DBG("%s [%d|%08x|%d|%02X %02X %02X %02X %02X %02X %02X]",msg,bus,id,len,data[0],data[1],data[2],data[3],data[4],data[5],data[6]);
		break;
	case 8:
		DBG("%s [%d|%08x|%d|%02X %02X %02X %02X %02X %02X %02X %02X]",msg,bus,id,len,data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
		break;
	}
}

}
