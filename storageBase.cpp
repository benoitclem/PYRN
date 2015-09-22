
#include "storageBase.h"
#include "MyMemoryAllocator.h"
#include "Configs.h"

#define __DEBUG__ STORAGE_BASE_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "storageBase.cpp"
#endif
#include "MyDebug.h"

extern MyMemoryAllocator memAlloc;

ramStorage::ramStorage(int sz) {
	bSz = sz;
	b = (char*) memAlloc.malloc(sz);
	memset(b,0x00,sz);
}

ramStorage::~ramStorage(void){
	bSz = 0;
	memAlloc.free(b);
}

int ramStorage::Read(char *buffer, int index, int length, int offset){
	if(index == 0) {
		memcpy(buffer,&sz,sizeof(int));
		return sizeof(int);
	} else if(index == 1) {
		memcpy(buffer,b,length);
		return length;
	} else {
		DBG("ramStorage: Read - index (%d) is incorrect",index);
	}
	return -1;
}

int ramStorage::Write(char *buffer, int index, int length, int offset){
	if(index == 0) {
		sz = *((int*)buffer);
		return sizeof(int);
	} else if(index == 1) {
		memcpy(b,buffer,length);
		return length;
	}
	return -1;
}

int ramStorage::Clear(int index) {
	if(index == 0) {
		sz = 0;
	} else if(index == 1) {
		memset(b,0,bSz);
	}
	return 0;
}

ramCircBuff::ramCircBuff(int sz) {
	currSz = 0;
	bSz = sz;
	pWrite = 0;
	pRead = 0;
	b = (char*) memAlloc.malloc(sz);
	memset(b,0x00,sz);
}

ramCircBuff::~ramCircBuff(void){
	currSz = 0;
	bSz = 0;
	pWrite = 0;
	pRead = 0;
	memAlloc.free(b);
}

int ramCircBuff::Put(char *buffer, int length) {
	// The buffer can receive all data?
	DBG("BEFORE - State %d - %d - %d",currSz,pRead,pWrite);
	if((currSz+sizeof(int)+length)<=bSz) {
		DBG("Enough space");
		// The buffer must be splitted ?
		if((pWrite+sizeof(int)+length)<=bSz) {
			DBG("No split");
			// Write header
			DBG("Write header");
			memcpy(b+pWrite,(char*)&length,sizeof(int));
			// Write data
			DBG("Write data");
			memcpy(b+pWrite+sizeof(int),buffer,length);
			currSz += (sizeof(int) + length);
			pWrite += (sizeof(int) + length);
			DBG("State %d - %d - %d",currSz,pRead,pWrite);
			// The write pointer must be resetted
			if(pWrite>=bSz) {
				DBG("Rewind W Pointer");
				pWrite = 0;
			}
		} else {
			DBG("Split somewhere");
			// The header must be splitted?
			if((pWrite+sizeof(int))<=bSz){
				DBG("No Split in header");
				// Write entire header
				memcpy(b+pWrite,(char*)&length,sizeof(int));
				pWrite += sizeof(int);
			} else {
				DBG("Split in header");
				// Divide header
				int sh = bSz - pWrite;
				memcpy(b+pWrite,(char*)&length,sh);
				memcpy(b,((char*)&length)+sh,sizeof(int)-sh);
				pWrite = sizeof(int)-sh;
			}
			currSz += sizeof(int);
			if((pWrite+length)<bSz){
				DBG("No Split in data");
				// Write entire data
				memcpy(b+pWrite,buffer,length);
				pWrite += length;
			} else {
				DBG("Split in data");
				// Divie data
				int s = bSz-pWrite; 
				memcpy(b+pWrite,buffer,s);
				memcpy(b,buffer+s,length-s);
				pWrite = length-s;
			}
			currSz += length;
			DBG("PUT AFTER - State %d - %d - %d",currSz,pRead,pWrite);
		}
		return length;
	}
	return -1;
}

int ramCircBuff::Get(char *buffer, int length) {
	// Data in buffer?
	DBG("GET BEFORE - State %d - %d - %d",currSz,pRead,pWrite);
	if(currSz) {
		DBG("Some data");
		int s = 0;
		// The header is splitted?
		if((pRead+sizeof(int))<=bSz) {
			DBG("Header is not splitted");
			memcpy(&s,b+pRead,sizeof(int));
			currSz -= sizeof(int);
			pRead += sizeof(int); 
			if(pRead>=bSz) {
				pRead = 0;
			}
		} else {
			DBG("Header is splitted");
			int l = bSz - pRead;
			memcpy((char*)&s,b+pRead,l);
			memcpy(((char*)&s)+l,b,sizeof(int)-l);
			currSz -= sizeof(int);
			pRead = sizeof(int)-l; 
		}
		// Assert the size of retrieve data
		if(s >256){
			DBG("Something wrong appened");
			DBG_MEMDUMP("internalBuffer",b,bSz);
		}
		int assertedLength = (s <= length)?s:length; 
		DBG("Read DataSz = %d(%d)",assertedLength,s);
		// The data is splitted?
		if((pRead+s)<=bSz){
			DBG("Data is not splitted");
			memcpy(buffer,b+pRead,s);
			currSz -= assertedLength;
			pRead += assertedLength;
			if(pRead>=bSz) {
				pRead = 0;
			}
		} else {
			DBG("Data is splitted");
			int l = bSz - pRead;
			memcpy(buffer,b+pRead,l);
			memcpy(buffer+l,b,s-l);
			currSz -= assertedLength;
			pRead = s-l;
		} 
		DBG("GET AFTER - State %d - %d - %d",currSz,pRead,pWrite);
		return assertedLength;
	} else {
		return 0;
	}
}

int ramCircBuff::Probe() {
	// Data in buffer?
	if(currSz) {
		int s = 0;
		// The header is splitted?
		if((pRead+sizeof(int))<=bSz) {
			memcpy(&s,b+pRead,sizeof(int));
		} else {
			int l = bSz - pRead;
			memcpy((char*)&s,b+pRead,l);
			memcpy(((char*)&s)+l,b,sizeof(int)-l);
		}
		return s;
	}
	return 0;
}

float ramCircBuff::FillLevel() {
	return ((float)currSz)/((float)bSz);
}