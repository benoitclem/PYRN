
#include "WanModem.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "WANModem.cpp"
#endif
#include "MyDebug.h"

WanModem::WanModem(): initializer(USBHost::getHostInst()) {
    DBG("Instanciate WanDongle");
    dongle.addInitializer(&initializer);
}

void WanModem::Init(void) {
    if(!dongle.tryConnect()) {
        DBG("Wait 10s to modem restart properly");
        Thread::wait(10000);
        if(!dongle.tryConnect()) {
            DBG("After retry the dongle Could not be connected");
        } else {
            DBG("After retry the dongle got connected"); 
        }
    }  else {
        DBG("Dongle got connected"); 
    }
}
