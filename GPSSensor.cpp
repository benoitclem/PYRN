

#include "GPSSensor.h"

#define __DEBUG__ 2
#ifndef __MODULE__
#define __MODULE__ "GPSSensor.cpp"
#endif
#include "MyDebug.h"

bool gpsNewlineDetected;

void rxFullCallback(MODSERIAL_IRQ_INFO *q){
    DBG("OVF");
    MODSERIAL *serial = q->serial;
    serial->rxBufferFlush();
}

void rxCallback(MODSERIAL_IRQ_INFO *q) {
    MODSERIAL *serial = q->serial;
    if ( serial->rxGetLastChar() == '\n') {
        gpsNewlineDetected = true;
    }
}

// Well there is an issue in the RX Serial data, sometime multiples frames collides between themselves... 
// To debug when we gonna have GSP Signal 

GPSSensor::GPSSensor(PinName tx, PinName rx, uint32_t trackingTime, uint32_t idle, uint32_t stackSz, unsigned char* sp):
    MySensor(SENSOR_NAME_GPS, SENSOR_TYPE_GPS, idle, stackSz, sp),
    gps(tx,rx) {
    fixed = false;
    trackTime = trackingTime;
    lastImpact = 0;
    gpsNewlineDetected = false;
    gps.baud(9600);
    gps.rxBufferFlush();
    gps.txBufferFlush();
    WARN("Increase the baudrate to 57600");
    gps.printf("$PUBX,41,1,0007,0003,57600,0*2B\r\n");
    Thread::wait(1000);
    gps.baud(57600);
    WARN("Disabling all unused NMEA sentences");
    gps.printf("$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n");
    gps.printf("$PUBX,40,ZDA,0,0,0,0,0,0*44\r\n");
    gps.printf("$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n");
    gps.printf("$PUBX,40,GSV,0,0,0,0,0,0*59\r\n");
    gps.printf("$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n");
    //gps.printf("$PUBX,40,RMC,0,0,0,0,0,0*47\r\n");
    Thread::wait(500);
    gps.rxBufferFlush();
    gps.txBufferFlush();
    WARN("Setup Callbacks");
    gps.attach(&rxCallback, MODSERIAL::RxIrq);
    gps.attach(&rxFullCallback, MODSERIAL::RxOvIrq);
    InitResults(SENSOR_RESSZ_DEFAULT);
    WARN("Finish Init");
}

// Could we have a way to put the InitResultStatic/StoreLastImpact into the base class?

void GPSSensor::InitResultsStatic(void) {
    DBG("InitResultStatic have been defined");
    results.start = (uint8_t*)store;
    results.current = (uint8_t*)store;
    results.max = sizeof(store); //IMU_STORE_SIZE*sizeof(uint16_t);
}

void GPSSensor::Loop(void) {
    // We read all the available data as fast as possible 
    // but giving opportunity to be preampted (wait 100ms)
    // Doing this reading should avoid serial internal buffer to ovf
     Sample();
     StoreLastImpact();
     Thread::wait(idleTime);
}

void GPSSensor::StoreLastImpact(void) {
    if(fixed && NeedImpact()) {
        if((results.max - results.num)>=sizeof(gpsImpact)){
            DBG("===========> Storing New impact <===========");
            if(resultsMutex.trylock()) {
                memcpy((char*)results.current,(const char*)&impact,sizeof(gpsImpact));
                results.current += sizeof(gpsImpact);
                results.num += sizeof(gpsImpact);
                DBG("%p %d %d",results.current,results.num,sizeof(gpsImpact));
                resultsMutex.unlock();
            }
        } else
            ERR("No Space to Store results... Data will be overwrited at next Loop");
    } else {
        DBG("Nope, (%s)", (!fixed)?"not fixed":"no impact needed");
    }
}

bool GPSSensor::NeedImpact(void) {
    time_t now = time(NULL);
    DBG("%ld-%ld = %d > %d ",now,lastImpact,now-lastImpact,trackTime);
    if((now-lastImpact)>trackTime){
        DBG("NEED IMPACT");
        lastImpact = now;
        return true;
    }
    return false;
}

int GPSSensor::GetLine(void) {
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

void GPSSensor::Sample(void) {
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
            // DBG_MEMDUMP("GPSData",(const char*)recvBuff,len);
            
            for(unsigned int i = 0; i < len; i++) {
                // Insert all data to gpsParser
                if(gpsParser.encode(recvBuff[i])) {
                    newData = true;
                }
            }
            
            // Only take a sample when all the data for each epoch have been received
            if(newData && gpsParser.rmc_ready() && gpsParser.gga_ready()){
                long lat = 0, lon = 0, hdop = 0;
                unsigned long date = 0, time = 0;
                gpsParser.get_position(&lat,&lon);
                gpsParser.get_datetime(&date,&time);
                hdop = gpsParser.hdop();
                if((lat != TinyGPS::GPS_INVALID_ANGLE) && (lon != TinyGPS::GPS_INVALID_ANGLE)){
                    fixed = true;
                    impact.date = date;
                    impact.time = time/100;
                    impact.lat = lat;
                    impact.lon = lon;
                    if(hdop>0xffff) {
                        impact.hdop = -1;
                    } else {
                        impact.hdop = hdop;
                    }
                    WARN("######[%ld-%ld][%09ld|%09ld|%04d]######",impact.date,impact.time,impact.lat,impact.lon,impact.hdop);
                } else {
                    fixed = false;
                    DBG("All data have been received but lat/lon does not seems to be valid",date,time,lat,lon,hdop);
                }
                gpsParser.reset_ready();
            } else {
                fixed = false;
                DBG("All frames did not came yet RMC(%c)GGA(%c)newData(%c)",gpsParser.rmc_ready()?'x':'o',gpsParser.gga_ready()?'x':'o',newData?'x':'o');
            }
        }
    }
}

// OLD SAMPLING 
/*
void GPSSensor::Sample(void) {
    int lineParsed = 0;
    DBG("Read GPS");
    if(gpsNewlineDetected) {
        gpsNewlineDetected = false;
        while(1) {
            int nChars = gps.rxBufferGetCount();
            if(nChars == 0) {
                DBG("No data to read anymore on gps line");
                break;
            } else {
                DBG("--> %d",nChars);
            }
                
            DBG("Read Line GPS");
            GetLine();
            DBG("GotLine, %s",recvBuff);
            DBG_MEMDUMP("GPSData",(const char*)recvBuff,strlen((const char*)recvBuff));
            
            // Stuffs used in GPGGA
            
            // The only NMEA sentence we gonna have (all other are disabled)
            int nArgs = sscanf((const char*)recvBuff, "GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c",&hour,&lat,&ns,&lon,&ew,&q,&su,&hdop,&alt,&altU);
            if(nArgs == 10) {
                DBG("We got all the arguments");
                DBG("%d [%f/%f/%c/%f/%c/%d/%d/%f/%f/%c]\n",nArgs,hour,lat,ns,lon,ew,q,su,hdop,alt,altU);
                lineParsed = GGA;
            }
            
            // if lineParsed is different from zero meens that's we got new data
            if(lineParsed) {
                DBG("Impact Result %d", su);
                // When 0 satelites usualy this we can't lock
                if(su == 0)
                    q = 0;
                
                if(q!=0){
                    DBG("Locked (q = %d)",q);
                    fixed = true;
                    impact.utc = (uint32_t) hour;
                    impact.lat = DegToDec(lat,ns);
                    impact.lon = DegToDec(lon,ew);
                    impact.alt = alt;
                    impact.su = su;
                    impact.hdop = hdop;
                    DBG("Impact Results %d, %f, %f, %f, %d, %d", impact.utc, impact.lat, impact.lon, impact.alt, impact.su, impact.hdop);
                }
            }
        }
    }
}
*/
 
