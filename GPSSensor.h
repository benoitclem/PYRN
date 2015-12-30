#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

#include "mbed.h"
#include "MySensor.h"
#include "MODSERIAL.h"
#include "TinyGPS.h"
#include "Configs.h"

//#define GPS_STORE_SIZE 32

class GPSSensor: public MySensor {
public:
    typedef struct _gpsImpact{
        uint32_t date;
        uint32_t time;
        int32_t lon;
        int32_t lat;
        int32_t alt;
        uint16_t hdop;
    }  __attribute__((packed)) gpsImpact;
protected:
    TinyGPS gpsParser;
    enum frameType{
        GGA = 1,
        RMC = 2
    };
    uint8_t recvBuff[GPS_RECV_BUFF];
    uint8_t store[GPS_STORE_SIZE];
    gpsImpact impact;
    MODSERIAL gps;
    //Serial gps;
    uint32_t trackTime;
    uint32_t lastImpact;
    bool fixed;
public:
    GPSSensor(PinName tx, PinName rx, uint32_t trackingTime = 10, uint32_t idle = 1000, uint32_t stackSz = GPS_THREAD_STACK_SIZE, unsigned char* sp = NULL);
    virtual void InitResultsStatic(void);
    virtual void Loop(void);
    virtual void StoreLastImpact(void);
    virtual bool GetImpact(GPSSensor::gpsImpact *pdata);
    virtual bool NeedImpact(void);
    virtual int GetLine(void);
    virtual void Sample(void);
};

#endif //GPS_SENSOR_H
