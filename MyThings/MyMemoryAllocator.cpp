
#include "MyMemoryAllocator.h"

#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "MyMemoryAllocator.cpp"
#endif
#include "MyDebug.h"

//#define ALLOC_MEM_SIZE 8192
#define ALLOC_MEM_SIZE 15104

char memory[ALLOC_MEM_SIZE] __attribute((section("AHBSRAM0")));
MyMemoryAllocator memAlloc(memory,ALLOC_MEM_SIZE);

MyMemoryAllocator::MyMemoryAllocator(char *m, uint16_t sz) {
	memory = m;
	memorySize = sz;
	head = (MemBlock*) memory;
	// initialise the linked list
	head->prev = NULL;
	head->sz = memorySize - sizeof(MemBlock);
	head->data = (char*)head + sizeof(MemBlock);
	head->status = FREE;
	head->next = NULL;
}

void MyMemoryAllocator::PrintResume(bool full) {
	MemBlock *pBlock = head;	
	uint16_t i = 0;
	uint16_t sum = 0;
	if(full) {
		DBG("START %p[%d]",pBlock,memorySize);
		while(pBlock) {
			sum += pBlock->sz;
			DBG("%d - %p - %d - %p - %p - %s",i++,pBlock, pBlock->sz, pBlock->prev, pBlock->next, (pBlock->status==FREE)?"FREE":"INUSE"); 
			pBlock = pBlock->next;
		}
		if(sum>memorySize) {
			DBG("MemoryAllocator fault %d>%d __________STOP__________",sum,memorySize);
			wait(1000);
			NVIC_SystemReset();
		}
		DBG("END");
	} else {
		while(pBlock) {
			sum += pBlock->sz;
			pBlock = pBlock->next;
		}
		DBG("MemoryAllocator status [%05d/%05d]",sum,memorySize);
	}	
}

// When getting out of this function pblock is still valid
void MyMemoryAllocator::JoinNextBlock(MemBlock *pBlock){
	MemBlock *nextBlock = pBlock->next;
	if(nextBlock) {
		// unlink the nextBlock
		pBlock->next = nextBlock->next;
		if(nextBlock->next)
			nextBlock->next->prev = pBlock;
		pBlock->sz += nextBlock->sz + sizeof(MemBlock);
	}
}

// When getting out of this function pblock is not valid anymore
void MyMemoryAllocator::JoinPrevBlock(MemBlock *pBlock) {
	MemBlock *prevBlock = pBlock->prev;
	if(prevBlock) {
		JoinNextBlock(prevBlock);
	}
}

MemBlock *MyMemoryAllocator::findBlock(char *p) {
	DBG("findBlock %p",p);
	MemBlock *pBlock = head;
	while(pBlock){
		if(pBlock->data == p) {
			return pBlock;
		}
		pBlock = pBlock->next;
	}
	DBG("Trying to free data that is not handler by memory allocator .... this is bad");
	return NULL;
}

void* MyMemoryAllocator::malloc(uint16_t sz) {
	MemBlock *pBlock = head;
	DBG("Malloc %d - %d",sz,sz+sz%4);
	// Align
	sz += sz%4;
	// lock access to shared ressource
	memAccess.lock();
	// Alloc
	while(pBlock){
		if(pBlock->status == FREE){
			if(pBlock->sz > (sz+sizeof(MemBlock))){
				// Fragment block
				MemBlock *nextBlock = (MemBlock*)(pBlock->data + sz);
				if(pBlock->next)
					pBlock->next->prev = nextBlock;
				nextBlock->sz = pBlock->sz - (sizeof(MemBlock) + sz);
				nextBlock->data = (char*)nextBlock + sizeof(MemBlock);
				nextBlock->status = FREE;
				nextBlock->prev = pBlock;
				nextBlock->next = pBlock->next;
				// Update block
				pBlock->sz = sz;
				pBlock->status = INUSE;
				pBlock->next = nextBlock;
				memAccess.unlock();
				PrintResume();
				return (void*)pBlock->data;
			}
		}
		pBlock = pBlock->next;
	}
	memAccess.unlock();
	PrintResume();
	return NULL;	
}

void MyMemoryAllocator::free(void *p) {
	DBG("free %p",p);
	// lock access to shared ressource
	memAccess.lock();
	// Find the block corresponding to allocated pointer
	MemBlock *pBlock = findBlock((char*)p);
	if(pBlock) {
		DBG("Found Block %p", pBlock);
		// Update Block
		pBlock->status = FREE;
		// Look after if we can join
		MemBlock *nextBlock = pBlock->next;
		if(nextBlock) {
			if(nextBlock->status == FREE) {
				DBG("Next is empty");
				JoinNextBlock(pBlock);
			}
		}
		PrintResume();
		// Look before if we can join
		DBG("Found Block %p", pBlock);
		MemBlock *prevBlock = pBlock->prev;
		if(prevBlock) {
			if(prevBlock->status == FREE) {
				DBG("Prev is empty");
				JoinPrevBlock(pBlock);
			}
		}
	}
	memAccess.unlock();
}

// Placement new operator
void* MyMemoryObject::operator new(size_t sz, void* v) {
    DBG("Placement New instanciation");
    return v;
}

// Placement new operator
void* MyMemoryObject::operator new(size_t sz) {
    DBG("New instanciation %d",sz);
    void *v = memAlloc.malloc(sz);
    if(v == NULL) {
    	DBG("Could not alloc memory for CANDiagCalculator");
    }
    return v;
}

void  MyMemoryObject::operator delete(void* ptr) {
	DBG("Delete instance");
	memAlloc.free(ptr);
}
