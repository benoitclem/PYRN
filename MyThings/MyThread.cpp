
#include "MyThread.h"
#include "MyLibc.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "MyThread.cpp"
#endif
#include "MyDebug.h"


void MainTrampoline(void const *args){
    MyThread *mt = (MyThread*) args;
    mt->Main();
    // Signalisation
    mt->sigValue = ENDSIG;
    mt->signal.put(&mt->sigValue);
    DBG("[%s] ENDSIG Sended",mt->tName);
    //mt->t->terminate();
}

MyThread::MyThread(const char* name, uint32_t sz, unsigned char *sp){
    stackSize = sz;
    stackPointer = sp;
    if(strlen(name) < THREADNAME_MAX_SZ)
    	strcpy(tName,name);
    else {
        DBG("Thread name is too long, truncate it");
    	strncpy(tName,name,THREADNAME_MAX_SZ-1);
    	tName[THREADNAME_MAX_SZ-1] = '\0';
    }
}

MyThread::~MyThread() {
}

void MyThread::Start(void){
    running = true;
}

void MyThread::Stop(void){
    running = false;
}

void MyThread::Run(void){
    t = new Thread(MainTrampoline,this,osPriorityNormal,stackSize,stackPointer);
}

void MyThread::Wait(int32_t ms){
    t->wait(ms);
}

void MyThread::WaitEnd(){
	osThreadId id = t->gettid();
	DBG("[%s] ENDSIG Wait",tName);
	// Signalisation
	while(1) {
		osEvent evt = signal.get();
		if (evt.status == osEventMessage) {
			uint32_t *sig = (uint32_t*)evt.value.p;
			if(*sig == ENDSIG) {
				DBG("[%s] ENDSIG Received",tName);
				break;
			}
		}
	}
   
	t->terminate();
}
