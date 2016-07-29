#ifndef LORA_MODEM_H
#define LORA_MODEM_H

#include "mbed.h"
#include "MODSERIAL.h"

class LoRaModem {
	MODSERIAL modem;
public:
	LoRaModem(PinName tx, PinName rx);
	virtual bool SendPayload(char *pyld, uint8_t size);
}

#endif // LORA_MODEM_H
