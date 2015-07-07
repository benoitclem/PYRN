/* PPPIPInterface.cpp */
/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#define __DEBUG__ 3
#ifndef __MODULE__
#define __MODULE__ "PPPIPInterface.cpp"
#endif
#include "MyDebug.h"

#include "core/errors.h"
#include "core/fwk.h"
#include "core/IOStream.h"
#include "rtos.h"

#include <cstdio>
using std::sscanf;

#include "PPPIPInterface.h"
#include "USBSerialStream.h"

#define PPP_SERIAL_PORT 0

#define MSISDN "*99#"
#define CONNECT_CMD "ATD " MSISDN "\x0D"
#define EXPECTED_RESP CONNECT_CMD "\x0D" "\x0A" "CONNECT" "\x0D" "\x0A"
#define EXPECTED_RESP_DATARATE CONNECT_CMD "\x0D" "\x0A" "CONNECT %d" "\x0D" "\x0A"
#define EXPECTED_RESP_MIN_LEN 20
#define OK_RESP "\x0D" "\x0A" "OK" "\x0D" "\x0A"
#define ESCAPE_SEQ "+++"
#define HANGUP_CMD "ATH" "\x0D"
#define NO_CARRIER_RESP "\x0D" "\x0A" "NO CARRIER" "\x0D" "\x0A"

extern "C" {
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/err.h"
//#include "lwip/dns.h"
#include "lwip/tcpip.h"
#include "netif/ppp/ppp.h"
}

PPPIPInterface::PPPIPInterface(USBSerialStream* pStream): 
    pppStream(pStream),
    pppSession(0),
    ipInitiated(false),
    pppInitiated(false),
    connected(false) {}
PPPIPInterface::~PPPIPInterface() { }

void PPPIPInterface::stackInits(const char* user, const char* pw)  {
    // LwIP Stuffs
    if(!ipInitiated) {
        DBG("LwIP: Init Stack");
        tcpip_init(PPPIPInterface::tcpipInitDoneCb, this);
        DBG("LwIP: Wait for setup");
        while (!ipInitiated)
            Thread::wait(1);
        DBG("LwIP: setup done");
    }
    // PPP Stuffs
    if(!pppInitiated) {
		DBG("LwIP: PPP Init");
        pppInit();
        pppSetAuth(PPPAUTHTYPE_ANY, user, pw);
        pppInitiated = true;
    }
}

int PPPIPInterface::dial(void) {
    int ret;
    char buf[32];
    size_t len;
    
    DBG("Sending %s", CONNECT_CMD);
    ret = pppStream->write((uint8_t*)CONNECT_CMD, strlen(CONNECT_CMD), osWaitForever);
    if( ret != OK )
        return NET_UNKNOWN;
    DBG("Expect %s", EXPECTED_RESP);
    
    /*
    size_t readLen;
    ret = pppStream->read((uint8_t*)buf, &readLen, EXPECTED_RESP_MIN_LEN, 10000);
    if( ret != OK )
        return NET_UNKNOWN;
    
    DBG("Readed %d chars", readLen);
    DBG_MEMDUMP("PPPAT",buf,readLen);
    */
    
    len = 0;
    size_t readLen;
    ret = pppStream->read((uint8_t*)buf + len, &readLen, EXPECTED_RESP_MIN_LEN, 10000);
    if( ret != OK )
        return NET_UNKNOWN;
        
    len += readLen;
    while( (len < EXPECTED_RESP_MIN_LEN) || (buf[len-1] != LF) ) {
        ret = pppStream->read((uint8_t*)buf + len, &readLen, 1, 10000);
        if( ret != OK )
            return NET_UNKNOWN;
        len += readLen;
    }

    buf[len]=0;

    DBG("Got %s[len %d]", buf, len);

    int datarate = 0;
    if( (sscanf( buf, EXPECTED_RESP_DATARATE, &datarate ) != 1) && (strcmp(EXPECTED_RESP, buf) != 0) ) {
        //Discard buffer
        do { //Clear buf
            ret = pppStream->read((uint8_t*)buf, &len, 32, 0);
            DBG("Got %s[len %d]", buf, len);
        } while( (ret == OK) && (len > 0) );
        return NET_CONN;
    }

    DBG("Transport link open");
    if(datarate != 0)
        DBG("Datarate: %d bps", datarate);
    return OK;
}

int PPPIPInterface::escape(void) {
    DBG("Sending %s", ESCAPE_SEQ);
    Thread::wait(1000);
    int ret = pppStream->write((uint8_t*)ESCAPE_SEQ, strlen(ESCAPE_SEQ), osWaitForever);
    Thread::wait(1000);
    if( ret != OK )
      return NET_UNKNOWN;
    return OK;
}

int PPPIPInterface::connect(const char* user, const char* pw) {
    
    DBG("Trying to connect with PPP");
    
    // Init the lWIP Stacks
    stackInits(user,pw);
    
    // Clear the line
    cleanupLink();
    
    // Do the dialing
    dial();
    
    // Launch the ppp session
    pppSession = pppOverSerialOpen(this, PPPIPInterface::linkStatusCb, this);
    DBG("PPP over serial opening returned = %d", pppSession);
    if (pppSession >= 0) {
        // the thread was successfully started.
        int retries = 600; // Approx (600*100ms) = 60s
        while (!connected && (retries-- > 0)) {
            Thread::wait(100);
        }
        
        if (retries!=0) {
            // We are connected on lwIP over PPP!
            DBG("Got Connected");
            return OK;
        } else {
            DBG("Got a Timeout");
            return NET_UNKNOWN;
        }    
    }else {
        DBG("Could not get ppp session");
        return NET_UNKNOWN;
    }
}

int PPPIPInterface::disconnect() {
    DBG("Do something on ppp stack and close the session");
    pppClose(pppSession);
    escape();
    cleanupLink();
    return OK;
}

void PPPIPInterface::setConnected(bool val) {
    connected = val;
}

bool PPPIPInterface::isConnected(void) {
    return connected;
}

int PPPIPInterface::cleanupLink() {
    int ret;
    char buf[32];
    size_t len;

    do { //Clear buf
        ret = pppStream->read((uint8_t*)buf, &len, 32, 100);
        if(ret == OK) {
            buf[len] = '\0';
            DBG("Got %s", buf);
        }
    } while( (ret == OK) && (len > 0) );
    
    DBG("Sending %s", HANGUP_CMD);
    
    // Here we could need to pass the ATStream to hangup
    IOStream *hangupPort = pppStream;   
    ret = hangupPort->write((uint8_t*)HANGUP_CMD, strlen(HANGUP_CMD), osWaitForever);
    if( ret != OK )
        return NET_UNKNOWN;

    size_t readLen;

    //Hangup
    DBG("Expect %s", HANGUP_CMD);
    
    len = 0;
    while( len < strlen(HANGUP_CMD) ) {
        ret = hangupPort->read((uint8_t*)buf + len, &readLen, strlen(HANGUP_CMD) - len, 100);
        if( ret != OK )
            break;
        len += readLen;
        /////
        buf[len]=0;
        DBG("Got %s", buf);
    }

    buf[len]=0;

    DBG("Got %s[len %d]", buf, len);

    //OK response
    DBG("Expect %s", OK_RESP);

    len = 0;
    while( len < strlen(OK_RESP) ) {
        ret = hangupPort->read((uint8_t*)buf + len, &readLen, strlen(OK_RESP) - len, 100);
        if( ret != OK ) {
            break;
        }
        len += readLen;
        /////
        buf[len]=0;
        DBG("Got %s", buf);
    }    

    buf[len]=0;

    DBG("Got %s[len %d]", buf, len);
    
    //NO CARRIER event
    DBG("Expect %s", NO_CARRIER_RESP);

    len = 0;
    while( len < strlen(NO_CARRIER_RESP) ) {
        ret = pppStream->read((uint8_t*)buf + len, &readLen, strlen(NO_CARRIER_RESP) - len, 100);
        if( ret != OK ) {
            break;
        }
        len += readLen;
        /////
        buf[len]=0;
        DBG("Got %s", buf);
    }

    buf[len]=0;

    DBG("Got %s[len %d]", buf, len);

    do { //Clear buf
        ret = pppStream->read((uint8_t*)buf, &len, 32, 100);
        if(ret == OK) {
            buf[len] = '\0';
            DBG("Got %s", buf);
        }
    } while( (ret == OK) && (len > 0) );

    return OK;
}

// IP Stuffs
char *PPPIPInterface::getIPAddress(void) {
    return ipAddress;
}

void PPPIPInterface::setIPAddress(char *ip) {
    strcpy(ipAddress,ip);
}

// PPP Stuffs
extern "C" {
    u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len) {
        //DBG("LEN %d",len);
        //DBG_MEMDUMP("IN",(const char*)data,len);
        PPPIPInterface* pIf = (PPPIPInterface*)fd;
        USBSerialStream *s = pIf->pppStream;
        int ret = s->write(data, len, osWaitForever); //Blocks until all data is sent or an error happens
        //DBG("sio_write OUT");
        if(ret != OK) {
            return 0;
        }
        return len;
    }
    
    u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len) {
        //DBG("sio_read");
        PPPIPInterface* pIf = (PPPIPInterface*)fd;
        size_t readLen;
        int ret = pIf->pppStream->read(data, &readLen, len, osWaitForever); //Blocks until some data is received or an error happens
        if(ret != OK) {
            return 0;
        }
        return readLen;
    }

    void sio_read_abort(sio_fd_t fd) {
        //DBG("sio_read_abort");
        PPPIPInterface* pIf = (PPPIPInterface*)fd;
        pIf->pppStream->abortRead();
    }
}


// LwIP Callbacks
void PPPIPInterface::tcpipInitDoneCb(void *ctx) {
    PPPIPInterface *pIf = (PPPIPInterface*)ctx;
    pIf->ipInitiated = true;
}

void PPPIPInterface::linkStatusCb(void *ctx, int errCode, void *arg) { 
    PPPIPInterface* pIf = (PPPIPInterface*)ctx;
    struct ppp_addrs* addrs = (struct ppp_addrs*) arg;

    switch(errCode) {
        case PPPERR_NONE:
            WARN("Connected via PPP.");
            INFO("Local IP address: %s", inet_ntoa(addrs->our_ipaddr));
            INFO("Netmask: %s", inet_ntoa(addrs->netmask));
            INFO("Remote IP address: %s", inet_ntoa(addrs->his_ipaddr));
            INFO("Primary DNS: %s", inet_ntoa(addrs->dns1));
            INFO("Secondary DNS: %s", inet_ntoa(addrs->dns2));
            //Setup DNS
            if (addrs->dns1.addr != 0) {
                //dns_setserver(0, (struct ip_addr*)&(addrs->dns1));
            }
            if (addrs->dns2.addr != 0) {
                //dns_setserver(1, (struct ip_addr*)&(addrs->dns1));
            }

            pIf->setConnected(true);
            pIf->setIPAddress(inet_ntoa(addrs->our_ipaddr));
            break;
        case PPPERR_CONNECT: //Connection lost
            WARN("Connection lost/terminated");
            pIf->setConnected(false);
            break;
        case PPPERR_AUTHFAIL: //Authentication failed
            WARN("Authentication failed");
            pIf->setConnected(false);
            break;
        case PPPERR_PROTOCOL: //Protocol error
            WARN("Protocol error");
            pIf->setConnected(false);
            break;
        case PPPERR_USER:
            WARN("Disconnected by user");
            pIf->setConnected(false);
            break;
        default:
            WARN("Unknown error (%d)", errCode);
            pIf->setConnected(false);
            break;
    }

    //pIf->m_linkStatusSphre.wait(0); //If previous event has not been handled, "delete" it now
    //pIf->m_pppErrCode = errCode;
    //pIf->m_linkStatusSphre.release();
}
