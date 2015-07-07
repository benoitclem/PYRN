
#ifndef PYRN_USB_MODEM_H
#define PYRN_USB_MODEM_H

#include "mbed.h"

#include "USBHost/USBHost3GModule/WANDongle.h"
#include "USBSerialStream.h"
#include "ATCommandsInterface.h"
#include "PPPIPInterface.h"

#include "HuaweiE372DongleInitializer.h"

class PyrnUSBModem {
private:
    HuaweiE372DongleInitializer initialiser;
    WANDongle dongle;
    
    USBSerialStream atStream;
    USBSerialStream pppStream;
    
    ATCommandsInterface at;
    PPPIPInterface ppp;
    
    bool atOpen;
    bool simReady;
    bool pppOpen;
    bool ipInit;  
public:
    PyrnUSBModem();
    bool init();
    
    WANDongleSerialPort *getAtInterface(int i);

    bool attached(void);
    bool pppConnected(void);

    int connect(const char* apn, const char* user, const char* password);
    int disconnect(void);

    char* getIPAddress(void);
};

#endif
