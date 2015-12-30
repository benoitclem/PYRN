
#include "CANVariableData.h"

#define __DEBUG__ CAN_VAR_DATA_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANVariableData.cpp"
#endif
#include "MyDebug.h"
#include "MyMemoryAllocator.h"

extern MyMemoryAllocator memAlloc;

CANVariableData::CANVariableData(int id, int allocSz, int typeSz) {
	tSz = typeSz;
	m = allocSz;
	n = sizeof(frameHdr);
	sz = 0;
	data = (char*) memAlloc.malloc(m);
	hdr = (frameHdr*) data;

	// set the static fields
	hdr->id = id;
	hdr->tSz = tSz;
}

void CANVariableData::AddData(char *d, int detSz) {
	if(n+tSz<m) {
		if (detSz == tSz) {
			memcpy(data+n,d,tSz);
			n += tSz;
			sz += tSz;
		} else {
			WARN("Detected Size is not the same ... dropping data");
		}
	} else {
		WARN("Buffer is full ... dropping data");
	}
}

void CANVariableData::GetNetworkFrame(char *buff) {
	// fill the content len
	hdr->sz = sz;
 	memcpy(buff,data,sz+sizeof(frameHdr));
 	n = sizeof(frameHdr);
 	sz = 0;
}