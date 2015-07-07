
#ifndef MY_MEMORY_ALLOCATOR_H
#define MY_MEMORY_ALLOCATOR_H

#include "mbed.h"
#include "rtos.h"

// formward declaration
typedef struct _MemBlock MemBlock;

typedef enum {
	INUSE,
	FREE,
} MemBlockStatus;

typedef struct _MemBlock {
	MemBlock *prev;
	uint16_t sz;
	char *data;
	MemBlockStatus status;
	MemBlock *next;
} MemBlock;

class MyMemoryAllocator {
protected:
	Mutex memAccess;
	char *memory;
	uint16_t memorySize;
	MemBlock *head;
public:
	MyMemoryAllocator(char *m, uint16_t sz);
	
	void PrintResume(void);
	void JoinNextBlock(MemBlock *pBlock);
	void JoinPrevBlock(MemBlock *pBlock);
	MemBlock *findBlock(char *p);
	
	void* malloc(uint16_t sz);
	void free(void *p);
};

class MyMemoryObject {
public:
	void* operator new(size_t sz, void* v);
    void* operator new(size_t sz);
    void  operator delete(void* ptr);
};

#endif
