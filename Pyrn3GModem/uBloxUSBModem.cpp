
#include "uBloxUSBModem.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "uBloxUSBModem.cpp"
#endif
#include "MyDebug.h"

class CPINProcessor : public IATCommandsProcessor {
public:
    enum PIN_STATUS { STATUS_NONE ,STATUS_SIMPIN, STATUS_SIMPUK, STATUS_READY };
    CPINProcessor() : status(STATUS_NONE) { }
    PIN_STATUS getStatus() { return status; }
private:
    virtual int onNewATResponseLine(ATCommandsInterface* pInst, const char* line) {
        DBG("GOT %s",line);
        if(!strncmp(line, "+CPIN: READY",12)) {
            status = STATUS_READY; 
            DBG("STATUS_READY");
        } else if(!strncmp(line, "+CPIN: SIM PIN",14)) {
            status = STATUS_SIMPIN;
            DBG("STATUS_SIMPIN");
        }else if(!strncmp(line, "+CPIN: SIM PUK",14)) {
            status = STATUS_SIMPUK;
            DBG("STATUS_SIMPUK");
        } else
            DBG("STATUS_NONE");
        return OK;
    }
    virtual int onNewEntryPrompt(ATCommandsInterface* pInst) { return OK; }
    volatile PIN_STATUS status;
};

class CREGProcessor : public IATCommandsProcessor {
public:
    enum PIN_STATUS { STATUS_REGISTERING, STATUS_OK, STATUS_FAILED };
    CREGProcessor() : status(STATUS_REGISTERING) { }
    PIN_STATUS getStatus() { return status; }
private:
    virtual int onNewATResponseLine(ATCommandsInterface* pInst, const char* line) {
        int r;
        if( sscanf(line, "+CREG: %*d,%d", &r) == 1 ) {
            switch(r) {
                case 1:
                case 5:
                    status = STATUS_OK;
                    break;
                case 0:
                case 2:
                    status = STATUS_REGISTERING;
                    break;
                case 3:
                default:
                    status = STATUS_FAILED;
                    break;
            }
        }
        return OK;
    }
    virtual int onNewEntryPrompt(ATCommandsInterface* pInst) { return OK; }
    volatile PIN_STATUS status;
};

// open drain behavior = 
// 			if 1  -> not drived (free behavior probably high due to modem internal pullup)
//			if 0  -> drived to 0	

// In our case SARA_U2
// RESET_N - > Reset occurs when low level appear on reset_n
// POWER_ON -> Power On / Off, pulse (1 - > 0(50us/80us) -> 1 ) 

uBloxUSBModem::uBloxUSBModem(PinName reset_pin_id,PinName pwrOn_pin_id) :
        initialiser(USBHost::getHostInst()),
        dongle(),
        atStream(dongle.getSerial(0)),
        pppStream(dongle.getSerial(1)),
        at(&atStream),
        ppp(&pppStream), 
		reset(reset_pin_id),
		pwrOn(pwrOn_pin_id)
 {
 	DBG("Instanciation ubx");
 	DBG("Add SARA U2 initializer");
    dongle.addInitializer(&initialiser);
 	reset.output();
 	reset.mode(OpenDrain);
 	reset = 1;	 // don't reset modem
 	pwrOn.output();
 	pwrOn.mode(OpenDrain);
 	pwrOn = 1;
 	//PulseOnOff();

 	simReady = false;
}

bool uBloxUSBModem::init(void) {
	if(!dongle.connected()) {
		while(1) {
			if(dongle.tryConnect())
				break;
			else 
				Thread::wait(1000);
		}
		DBG("Starting AT thread if needed");
        int ret = at.open();
        if(ret) {
            ERR("Opening AT failed");
            return false;
        }
    
        DBG("Sending initialisation commands");
        // Echo 1
        // Format CRLF
        // Unsollicited Codes disabled
        ret = at.init("AT+CMEE=2");
        if(ret != OK) {
            ERR("Initialisation AT failed");
            return false;
        } else {
        	DBG("Modem is ready to use");

        }

        ATCommandsInterface::ATResult atRes;
        at.executeSimple("AT+UGPIOC=16,2",&atRes);

        ATCommandsInterface::ATResult result;

	    if(!simReady){
	        // SIM PIN Stuff here
	        int retries = 3;
	        do {

	            CPINProcessor cpinProcessor;
	            INFO("Check CPIN STATE");
	            ret = at.execute("AT+CPIN?", &cpinProcessor, &result,5000);
	            //INFO("Result of command: Err code=%d", ret);
	            //INFO("ATResult: AT return=%d (code %d)", result.result, result.code);
	            if(result.code == 14) {
	                 WARN("SIM IS Busy retry");
	                 retries++;
	                 Thread::wait(500);
	            } else if(ret == OK) {
	                if(cpinProcessor.getStatus() == CPINProcessor::STATUS_READY) {
	                    INFO("ME PIN READY");
	                    simReady = true;
	                    break;
	                } else if (cpinProcessor.getStatus() == CPINProcessor::STATUS_SIMPIN)  {
	                    INFO("ME SIM PIN NEEDED");
	                    break;
	                    /*ret = at.executeSimple("AT+CPIN=\"0000\"",NULL,5000);
	                    if(ret != OK){
	                        ERR("CPIN ERROR ... do not retry");
	                        break;
	                    } else {
	                        INFO("CPIN OK");
	                    }*/
	                } else if (cpinProcessor.getStatus() == CPINProcessor::STATUS_SIMPUK) {
	                    INFO("CPIN IS PUKED");
	                    break;
	                } else {
	                    ERR("UNKNOWN STATUS");
	                    break;
	                }
	            } else {
	                INFO("SIM PIN ERROR: SIM Probably not inserted");
	                break;
	            }

	            retries--;
	        } while(retries);
	        
	        if(!simReady) {
	            ERR("Couldn't pin unlock ...");
	            return false;
	        }
	    } else {
	        INFO("SIM PIN have been unlocked somewhere!");
	    }

	        //Wait for network registration
	    CREGProcessor cregProcessor;
	    do {
	        INFO("Waiting for network registration");
	        ret = at.execute("AT+CREG?", &cregProcessor, &result);
	        //INFO("Result of command: Err code=%d", ret);
	        //INFO("ATResult: AT return=%d (code %d)", result.result, result.code);
	        if(cregProcessor.getStatus() == CREGProcessor::STATUS_REGISTERING) {
	            Thread::wait(1000);
	        }
	    } while(cregProcessor.getStatus() == CREGProcessor::STATUS_REGISTERING);
	    if(cregProcessor.getStatus() == CREGProcessor::STATUS_FAILED) {
	        ERR("Registration denied");
	        return false;
	    }
	}

	return true;
}

int uBloxUSBModem::connect(const char* apn, const char* user, const char* password) {
	int ret;
	
    if(!init()) {
        ERR("Modem could not register");
        return -1;
    }
       
    ATCommandsInterface::ATResult result;
	
    if(apn != NULL) {
        char cmd[48];
        int tries = 30;
        INFO("Setting APN to %s", apn);
        sprintf(cmd, "AT+CGDCONT=1,\"IP\",\"%s\"", apn);
        do {//Try 30 times because for some reasons it can fail *a lot* with the K3772-Z dongle
            ret = at.executeSimple(cmd, &result);
            //DBG("Result of command: Err code=%d", ret);
            if(ret)
                Thread::wait(500);
        } while(ret && --tries);
        // DBG("ATResult: AT return=%d (code %d)", result.result, result.code);
    }
        
    INFO("Connecting PPP");
    ret = ppp.connect(user, password);
    INFO("Result of connect: Err code=%d", ret);

    return ret;
}


void uBloxUSBModem::PulseOnOff(void) {
	DBG("PulseOnOff");
	pwrOn = 0;
	wait_ms(60); // Not threaded wait
	pwrOn = 1;
	wait_ms(60);
}