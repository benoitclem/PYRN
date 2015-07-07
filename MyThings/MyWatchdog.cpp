
#include "MyWatchdog.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "Watchdog.cpp"
#endif
#include "MyDebug.h"

// The Watchdog actually own no data,
// The class is just here as a convenient 
// way to enclose code
MyWatchdog::MyWatchdog(float s) {
	WDReboot();
	Feed(s);
}

bool MyWatchdog::WDReboot() {
    if((LPC_WDT->WDMOD >> 2) & 1) {
        DBG("Boot: WDT");
		return true;	
	} else {
		DBG("Boot: Normal");
		return false;
	}
}

// Load timeout value in watchdog timer and enable
void MyWatchdog::Feed(float s) {
    LPC_WDT->WDCLKSEL = 0x1;                // Set CLK src to PCLK
    uint32_t clk = SystemCoreClock / 16;    // WD has a fixed /4 prescaler, PCLK default is /4
    LPC_WDT->WDTC = s * (float)clk;
    LPC_WDT->WDMOD = 0x3;                   // Enabled and Reset
    Feed();
}

// "kick" or "feed" the dog - reset the watchdog timer
// by writing this required bit pattern
void MyWatchdog::Feed() {
    LPC_WDT->WDFEED = 0xAA;
    LPC_WDT->WDFEED = 0x55;
}
