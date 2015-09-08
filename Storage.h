
#ifndef STORAGE_H_
#define STORAGE_H_

#include "mbed.h"
#include "rtos.h"

#include "SDHCFileSystem.h"

typedef struct _StorageEntry {
	char name[16];
	Mutex *mutex;
	bool opened;
	FILE *fp;
} StorageEntry;

class Storage {
private:
	static Storage *storage;
	SDFileSystem sd;
	StorageEntry *repos;
	uint16_t nReps;
	uint16_t cnReps;
public:
	Storage(uint8_t nRepos);
	~Storage() ;
	StorageEntry* 	InstanciateRepository(StorageEntry *repo);
	void 			FreeRepository(StorageEntry *repo);
	StorageEntry*	SearchRepository(const char* repName);
	void 			OpenRepository(const char* repName, StorageEntry *repo);
	void 			CloseRepository(StorageEntry *repo);
	uint16_t 		WriteData(const char* repo, uint8_t *buff, uint16_t n, bool overwrite);
	uint16_t 		ReadDataSize(const char* repo);
	uint16_t 		ReadData(const char* repo, uint8_t *buff, uint16_t max, int16_t offset, bool erase);
	void 			daRemove(const char* file);
};

#endif /*STORAGE_H_*/
