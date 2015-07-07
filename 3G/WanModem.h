
#ifndef WAN_MODEM_H
#define WAN_MODEM_H

#include "WANDongle.h"
#include "MyThread.h"
#include "HuaweiE372DongleInitializer.h"
#include "USBSerialStream.h"
#include "ATCommandsInterface.h"

class WanModem {
    HuaweiE372DongleInitializer initializer;
    WANDongle dongle;
public:
    WanModem();
    virtual void Init(void);
};

#endif // WAN_MODEM_H