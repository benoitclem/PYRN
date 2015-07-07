
#ifndef MYTHREADS_H
#define MYTHREADS_H

#include "mbed.h"
#include "rtos.h"
 
#define ENDSIG				0x01
#define THREADNAME_MAX_SZ	32

/** MyThread class.
 *  Used to simplify the use of threads.
 *  This is a pure virtual class, subclass it.
 *  Example:
 *  @code
 *  class blinker: public MyThread {
 *  private:
 *      PwmOut *led;
 *      int32_t delay;
 *  public:
 *      blinker(PwmOut *o, int32_t d);
 *      virtual void Main(void);
 *  };
 *  @endcode 
 */
class MyThread{
public:
    char tName[THREADNAME_MAX_SZ];
    uint32_t stackSize;
    bool running;
    unsigned char *stackPointer;
    Thread *t;
    uint32_t sigValue;
    Queue<uint32_t, 1> signal;

    MyThread(const char* name, uint32_t stack_size = DEFAULT_STACK_SIZE, unsigned char *sp = NULL);
    virtual ~MyThread();
    virtual void Start();
    virtual void Stop();
    virtual void Run();
    virtual void Main(void) = 0;
    virtual void Wait(int32_t ms);
    virtual void WaitEnd();
};

#endif
