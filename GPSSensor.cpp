#include "GPSSensor.h"

#define __DEBUG__ GPS_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "GPSSensor.cpp"
#endif
#include "MyDebug.h"


GPSSensor::GPSSensor(PinName tx, PinName rx, uint32_t idle, uint32_t stackSz, unsigned char* sp):
    MySensor(SENSOR_NAME_GPS, SENSOR_TYPE_GPS, idle, stackSz, sp),
    gps(tx,rx) {
    InitResults(SENSOR_RESSZ_DEFAULT);
    fixed = false;
}

void GPSSensor::InitResultsStatic(void) {
    DBG("InitResultStatic have been defined");
    results.start = (uint8_t*)store;
    results.current = (uint8_t*)store;
    results.max = sizeof(store); //IMU_STORE_SIZE*sizeof(uint16_t);
}

void GPSSensor::Loop(void) {
    fixed = false; 
    Sample();
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