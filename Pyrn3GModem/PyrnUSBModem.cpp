
#include "PyrnUSBModem.h"

#define __DEBUG__ 3
#ifndef __MODULE__
#define __MODULE__ "PyrnUSBModem.cpp"
#endif
#include "MyDebug.h"

// Command Processors

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

class COPSProcessor : public IATCommandsProcessor {
public:
    COPSProcessor() : valid(false) {
        network[0] = '\0';
        apn[0] = '\0';
        bearer[0] = '\0';
    }
    char* getNetwork() { return network; }
    char* getAPN() { return apn; }
    char* getBearer() { return bearer; }
    bool isValid() { return valid; }
private:
    virtual int onNewATResponseLine(ATCommandsInterface* pInst, const char* line) {
        int networkId;
        int bearerId;
        int s = sscanf(line, "+COPS: %*d,%*d,\"%d\",%d", &networkId, &bearerId);
        if( s == 2 ) {
            switch(networkId) {
                case 23415:
                    strcpy(network, "Vodafone UK");
                    strcpy(apn, "pp.vodafone.co.uk");
                    valid = true;
                    break;
                case 20810:
                    strcpy(network, "SFR FR");
                    strcpy(apn, "websfr");
                    valid = true;
                    break;
                default:
                    break;
            }
        } else
            return OK;
        
        switch(bearerId) {
            case 0: strcpy(bearer, "GSM"); break;
            case 1: strcpy(bearer, "GSM Compact"); break;
            case 2: strcpy(bearer, "UTRAN"); break;
            case 3: strcpy(bearer, "GSM w/EGPRS"); break;
            case 4: strcpy(bearer, "UTRAN w/HSDPA"); break;
            case 5: strcpy(bearer, "UTRAN w/HSUPA"); break;
            case 6: strcpy(bearer, "UTRAN w/HSDPA and HSUPA"); break;
            case 7: strcpy(bearer, "E-UTRAN"); break;
            default: break;
        }
        return OK;
    }
    
    virtual int onNewEntryPrompt(ATCommandsInterface* pInst) { return OK; }
    
    char network[24];
    char bearer[24];
    char apn[24];
    volatile bool valid;
};

// ==================== MODEM =====================

PyrnUSBModem::PyrnUSBModem():
        initialiser(USBHost::getHostInst()),
        dongle(),   
        atStream(dongle.getSerial(3)),
        pppStream(dongle.getSerial(0)),
        at(&atStream),
        ppp(&pppStream), 
        atOpen(false),
        simReady(false),
        pppOpen(false),
        ipInit(false) {
    DBG("Add E372 dongle initializer");
    dongle.addInitializer(&initialiser);
}

bool PyrnUSBModem::init() {
    int ret = 0;

    if(!dongle.connected()){
        bool detectConnectedModem = false;
        for (int x=0; x<3;x++){
            INFO("Trying to connect the dongle");
            dongle.tryConnect();
            if (dongle.connected()) {
                DBG("Great the dongle is connected - I've tried %d times to connect", x);
                detectConnectedModem = true;
                break;                              // Break out of the for loop once the dongle is connected - otherwise try for a while more
            }
            Thread::wait(3500);
        }
        if (!detectConnectedModem) {
            // OK we got this far - so give up trying and let someone know you can't see the modem
            ERR("There is no dongle pluged into the board, or the module does not respond. Is the module/modem switched on?");
            return false;
        }
    } else {
        INFO("Dongle is already connected ... continue");
    }

    if(atOpen) {
        INFO("Stream is already opened go to SIM Check");
    } else {

        DBG("Starting AT thread if needed");
        ret = at.open();
        if(ret) {
            ERR("Opening AT failed");
            return false;
        }
    
        DBG("Sending initialisation commands");
        // Echo 1
        // Format CRLF
        // Unsollicited Codes disabled
        ret = at.init("ATZ E1 V1 ^CURC=0");
        if(ret) {
            ERR("Initialisation AT failed");
            return false;
        }
        
        if(dongle.getDongleType() == WAN_DONGLE_TYPE_HUAWEI_E372) {
            INFO("Using a Vodafone E372 Dongle");
            ERR("Send CMEE cmd ...");
            ret = at.executeSimple("AT+CMEE=1",NULL,5000);
            if(ret != OK) {
                ERR("CMEE cmd failed");
                return false;
            }
        } else  {
            WARN("Using an Unknown Dongle.. do specific init");
        }
        
        atOpen = true;

    }

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
                    ret = at.executeSimple("AT+CPIN=\"0000\"",NULL,5000);
                    if(ret != OK){
                        ERR("CPIN ERROR ... do not retry");
                        break;
                    } else {
                        INFO("CPIN OK");
                    }
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
	
    return true;
}

bool PyrnUSBModem::attached(void) {
    return dongle.connected();
}

bool PyrnUSBModem::pppConnected(void) {
    return ppp.isConnected();
}

/*
WANDongleSerialPort *PyrnUSBModem::getAtInterface(int i) {
    return dongle.getWANSerial(i);
}
*/

int PyrnUSBModem::connect(const char* apn, const char* user, const char* password) {
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

int PyrnUSBModem::disconnect() {

    INFO("Disconnecting from PPP");
    int ret = ppp.disconnect(); 
	if(ret) {
        ERR("Disconnect returned %d, still trying to disconnect", ret);
    }

    Thread::wait(500);

    return OK;
}

char* PyrnUSBModem::getIPAddress() {
    return ppp.getIPAddress();
}
