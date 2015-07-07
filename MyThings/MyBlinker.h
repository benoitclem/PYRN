
#ifndef BLINKER_H
#define BLINKER_H

#include "mbed.h"
#include "MyThread.h"

#define BLINKER_THREAD_STACK_SIZE   512

/** MyThread class.
 *  Create a self blinking led. 
 *  Use it to display some states.
 */
class MyBlinker: public MyThread {
private:
    PwmOut *led;
    int32_t delay;
    float highState;
    float lowState;
public:
    MyBlinker(PwmOut *o, int32_t d, float hState = 1.0, float lState = 0.0, uint32_t stackSz = BLINKER_THREAD_STACK_SIZE, unsigned char* sp = NULL);
    virtual void setDelay(int32_t d);
    virtual void setHighState(float hState);
    virtual void setLowState(float lState);
    virtual void Main(void);
};

#endif
