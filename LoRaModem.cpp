
#include "LoRaModem.h"

#define __DEBUG__ LORA_DBG_LVL
#ifndef __MODULE__
#define __MODULE__ "LoRaModem.cpp"
#endif
#include "MyDebug.h"

bool LoRaNewlineDetected;

void LoRaRxFullCallback(MODSERIAL_IRQ_INFO *q){
    DBG("OVF");
    MODSERIAL *serial = q->serial;
    serial->rxBufferFlush();
}

void LoRaRxCallback(MODSERIAL_IRQ_INFO *q) {
    MODSERIAL *serial = q->serial;
    //DBG("%c",serial->rxGetLastChar());
    if ( serial->rxGetLastChar() == '\n') {
        LoRaNewlineDetected = true;
    }
}

LoRaModem::LoRaModem(PinName tx, PinName rx): 
	modem(tx,rx) {
	DBG("Instanciate the LoRa Modem");
	LoRaNewlineDetected = false;
	modem.baud(57600);
    modem.rxBufferFlush();
    modem.txBufferFlush();
    gps.attach(&LoRaRxCallback, MODSERIAL::RxIrq);
    gps.attach(&LoRaRxFullCallback, MODSERIAL::RxOvIrq);
}

void LoRaModem::GetLine(char* buff, uint8_t *len, uint16_t timeout) {
	uint16_t cTime = 0;
	while(1) {
		if(!LoRaNewlineDetected)
			Thread::Wait(100);
			cTime += 100;
			if(cTime > timeout) {
				*len = 0;
				return;
			}
		else {
			int nChars = gps.rxBufferGetCount();
			if(nChars == 0) {
				*len = 0;
				return;
			} else {

			}
		}
	}

}

bool LoRaModem::Register(void) {
	DBG("Registering to network");
	for(int i = 0; i < size; i++) 
		modem.putc(b[i]);
}

bool LoRaModem::SendPayload(char *pyld, uint8_t size) {
	DBG_MEMDUMP("Send PayLoad",pyld,size);
	return true;
}
