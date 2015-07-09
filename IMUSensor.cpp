
#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "IMUSensor.cpp"
#endif
#include "MyDebug.h"

#include "IMUSensor.h"

#define IMUVER1		0x01

IMUSensor::IMUSensor(PinName sda, PinName scl, uint32_t idle): 
    MySensor(SENSOR_NAME_IMU, SENSOR_TYPE_IMU, idle, IMU_THREAD_STACK_SIZE, stack), 
    Accelerometer(sda,scl){
    InitResults(SENSOR_RESSZ_DEFAULT);
}

void IMUSensor::InitResultsStatic(void) {
    DBG("InitResultStatic have been defined");
    results.start = (uint8_t*)store;
    results.current = (uint8_t*)store;
    results.max = sizeof(store); //IMU_STORE_SIZE*sizeof(uint16_t);
}

void IMUSensor::Loop(void) {
    // Do the reading
    float lax,lay,laz,lmx,lmy,lmz;              // Locals
    Accelerometer.readAcc(&lax,&lay,&laz);
    Accelerometer.readMag(&lmx,&lmy,&lmz);
    DBG("[%+.4f,%+.4f,%+.4f] - [%+.4f,%+.4f,%+.4f]", lax,lay,laz,lmx,lmy,lmz);
    // Do the conversion to the impact struct
    impact.ver = IMUVER1;
    impact.t = time(NULL);
    impact.ax = (int16_t) (lax * 1000);
    impact.ay = (int16_t) (lay * 1000);
    impact.az = (int16_t) (laz * 1000);
    impact.mx = (int16_t) (lmx * 1000);
    impact.my = (int16_t) (lmy * 1000);
    impact.mz = (int16_t) (lmz * 1000);
    // Print result if debug is activated
    INFO("[%+05d,%+05d,%+05d] - [%+05d,%+05d,%+05d]", impact.ax, impact.ay, impact.az, impact.mx, impact.my, impact.mz);
}

void IMUSensor::StoreLastImpact(void) {
    if((results.max - results.num)>=sizeof(imuImpact)){
        if(resultsMutex.trylock()) {
            memcpy((char*)results.current,(const char*)&impact,sizeof(imuImpact));
            results.current += sizeof(imuImpact);
            results.num += sizeof(imuImpact);
            //DBG("%p %d %d",results.current,results.num,sizeof(imuImpact));
            resultsMutex.unlock();
        }
    } else
        WARN("No Space to Store results... Data will be overwrited at next Loop");
}
