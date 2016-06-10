#ifndef GPS_SENSOR_NMEA_H
#define GPS_SENSOR_NMEA_H

#include "mbed.h"
#include "GPSSensor.h"
#include "TinyGPS.h"
#include "Configs.h"

//#define GPS_STORE_SIZE 32

class GPSSensorNMEA: public GPSSensor {
protected:
    TinyGPS gpsParser;
    enum frameType{
        GGA = 1,
        RMC = 2
    };
    uint32_t lastImpact;
public:
    GPSSensorNMEA(PinName tx, PinName rx, uint32_t idle = 1000, uint32_t stackSz = GPS_THREAD_STACK_SIZE, unsigned char* sp = NULL);
    virtual bool GetImpact(GPSSensorNMEA::gpsImpact *pdata);
    virtual bool NeedImpact(void);
    virtual int GetLine(void);
    virtual void Sample(void);
};

#endif //GPS_SENSOR_NMEA_H
