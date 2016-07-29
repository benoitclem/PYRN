

#include "GPSSensorNMEA.h"

#define __DEBUG__ GPS_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "GPSSensorNMEA.cpp"
#endif
#include "MyDebug.h"

bool gpsNewlineDetected;

void nmeaRxFullCallback(MODSERIAL_IRQ_INFO *q){
    DBG("OVF");
    MODSERIAL *serial = q->serial;
    serial->rxBufferFlush();
}

void nmeaRxCallback(MODSERIAL_IRQ_INFO *q) {
    MODSERIAL *serial = q->serial;
    //DBG("%c",serial->rxGetLastChar());
    if ( serial->rxGetLastChar() == '\n') {
        gpsNewlineDetected = true;
    }
}

// Well there is an issue in the RX Serial data, sometime multiples frames collides between themselves... 
// To debug when we gonna have GSP Signal 

/* ## NMEA CheckSum
def cs(t):
    xor = 0
    for c in t:
        xor = xor ^ ord(c)
    return xor
*/

GPSSensorNMEA::GPSSensorNMEA(PinName tx, PinName rx, uint32_t idle, uint32_t stackSz, unsigned char* sp):
    GPSSensor(tx,rx,100,stackSz,sp) {
    lastImpact = time(NULL);
    trackTime = idle;
    gpsNewlineDetected = false;
    gps.baud(9600);
    gps.rxBufferFlush();
    gps.txBufferFlush();
    //WARN("Increase the baudrate to 57600");
    //gps.printf("$PUBX,41,1,0007,0003,57600,0*2B\r\n");
    //gps.baud(57600);
    //Thread::wait(500);
    gps.rxBufferFlush();
    gps.txBufferFlush();
    WARN("Setup Callbacks");
    gps.attach(&nmeaRxCallback, MODSERIAL::RxIrq);
    gps.attach(&nmeaRxFullCallback, MODSERIAL::RxOvIrq);
    WARN("Finish Init");
}

bool GPSSensorNMEA::GetImpact(GPSSensorNMEA::gpsImpact *pdata) {
    if(fixed) {
        memcpy((char*)pdata,(const char*)&impact,sizeof(gpsImpact));
        DBG_MEMDUMP("data",(char*)pdata,sizeof(gpsImpact));
        return true;
    }
    return false;
}

bool GPSSensorNMEA::NeedImpact(void) {
    time_t now = time(NULL)*1000;
    DBG("%ld-%ld = %d > %d ",now,lastImpact,now-lastImpact,trackTime);
    if((now-lastImpact)>trackTime){
        DBG("NEED IMPACT");
        lastImpact = now;
        return true;
    }
    return false;
    //return true;
}

int GPSSensorNMEA::GetLine(void) {
    int len = 0;
    // Consume the chars until we found a '$'
    while (gps.getc() != '$');
    recvBuff[0] = '$';
    // Fill the GPS_RECV_BUFF 
    for (int i=1; i<(GPS_RECV_BUFF-2); i++) {
        recvBuff[i] = gps.getc();
        len++;
        if (recvBuff[i] == '\n') {
            recvBuff[i+1] = '\0';
            return len;
        }        
    }
    error("Overflow in getline");
    return GPS_RECV_BUFF-2;
}

void GPSSensorNMEA::Sample(void) {
    int len = 0;
    bool newData = false;
    if(gpsNewlineDetected) {
        while(1) {
            int nChars = gps.rxBufferGetCount();
            if(nChars == 0) 
                break;
            //else
            //    DBG("To read --> %d",nChars);
            // We could consider passing the data directly to parser whithout buffering it
            // but: That's annoying cause if so we could not printf them smoothly on console
            len = GetLine();
            
            //DBG("GotLine(%d) %s",len,recvBuff);
            DBG_MEMDUMP("GPSData",(const char*)recvBuff,len);
            
            for(unsigned int i = 0; i < len; i++) {
                // Insert all data to gpsParser
                if(gpsParser.encode(recvBuff[i])) {
                    newData = true;
                }
            }
            
            // Only take a sample when all the data for each epoch have been received
            if(gpsParser.gll_ready()){
                long lat = 0, lon = 0, hdop = 0;
                unsigned long date = 0, time = 0;
                long alt = 0;
                gpsParser.get_position(&lat,&lon,&alt);
                gpsParser.get_datetime(&date,&time);
                hdop = gpsParser.hdop();
                if((lat != TinyGPS::GPS_INVALID_ANGLE) && (lon != TinyGPS::GPS_INVALID_ANGLE)){
                    fixed = true;
                    impact.date = date;
                    impact.time = time/100;
                    impact.lat = lat;
                    impact.lon = lon;
                    impact.alt = alt;
                    if(hdop>0xffff) {
                        impact.hdop = -1;
                    } else {
                        impact.hdop = hdop;
                    }
                    WARN("######[%ld-%ld][%08x - %09ld|%08x - %09ld|%04d]######",impact.date,impact.time,impact.lat,impact.lat,impact.lon,impact.lon,impact.hdop);
                } else {
                    fixed = false;
                    DBG("All data have been received but lat/lon does not seems to be valid",date,time,lat,lon,hdop);
                }
                gpsParser.reset_ready();
            } else {
                fixed = false;
            }
        }
    }
}