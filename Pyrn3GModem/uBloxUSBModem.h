
#ifndef UBLOX_USB_MODEM_H
#define UBLOX_USB_MODEM_H

#include "mbed.h"

#include "USBHost/USBHost3GModule/WANDongle.h"
#include "USBSerialStream.h"
#include "ATCommandsInterface.h"
#include "PPPIPInterface.h"

#include "SaraU2DongleInitializer.h"

class uBloxUSBModem {
private:
	SaraU2DongleInitializer initialiser;
	WANDongle dongle;

    USBSerialStream atStream;
    USBSerialStream pppStream;
    
    ATCommandsInterface at;
    PPPIPInterface ppp;

	DigitalInOut reset;
	DigitalInOut pwrOn;

	bool simReady;
public:
	uBloxUSBModem(PinName reset_pin_id,PinName pwrOn_pin_id);
    bool init(void);
    int connect(const char* apn, const char* user, const char* password);
	void PulseOnOff(void);
};

#endif 
