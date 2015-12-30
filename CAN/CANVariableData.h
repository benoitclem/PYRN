
#ifndef CAN_VARIABLE_DATA_H
#define CAN_VARIABLE_DATA_H

#include "mbed.h"

class CANVariableData {
private:
	typedef struct _frameHdr{
		uint32_t id;
		uint16_t sz;
		uint8_t	tSz;	
	} __attribute__((packed)) frameHdr;
protected:
	char *data;

	int n;
	int m;
	int sz;
	int tSz;
	frameHdr *hdr; // Point to data
public:
	CANVariableData(int id, int allocSz, int typeSz);
	void AddData(char *d, int detSz);
	int GetDataSz() {return sz;};
	char *GetData() {return data+sizeof(frameHdr);};
	void GetNetworkFrame(char *buff);
};



#endif // CAN_VARIABLE_DATA_H