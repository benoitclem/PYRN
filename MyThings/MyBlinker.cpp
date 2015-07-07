
#include "MyBlinker.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "MyDebug.cpp"
#endif
#include "MyDebug.h"


MyBlinker::MyBlinker(PwmOut *o, int32_t d, float hState, float lState, uint32_t stackSz, unsigned char* sp): MyThread("MyBlinker",stackSz,sp) {
    led = o;
    delay = d;
    highState = hState;
    lowState = lState;
    Start();
    Run();
}

void MyBlinker::setDelay(int32_t d) {
    delay = d;
}

void MyBlinker::setHighState(float hState) {
    highState = hState;
}

void MyBlinker::setLowState(float lState) {
    lowState = lState;
}

void MyBlinker::Main(void) {
    while(running){
        led->write(highState);
        Wait(delay);
        led->write(lowState);
        Wait(delay);
    }
    DBG("End MyBlinker");
}
