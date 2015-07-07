
#ifndef MY_WATCHDOG_H
#define MY_WATCHDOG_H

#include "mbed.h"

class MyWatchdog {
public:
	MyWatchdog(float s);
	bool WDReboot();    
	void Feed(float s);
    void Feed();
};

#endif
