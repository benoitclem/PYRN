#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include "mbed.h"
#include "MySensor.h"
#include "LSM303DLH.h"

#define IMU_STORE_SIZE      	128
#define IMU_THREAD_STACK_SIZE   512

class IMUSensor: public MySensor {
protected:
    uint16_t store[IMU_STORE_SIZE];
    typedef struct _imuImpact{
    	uint8_t	ver;
    	uint32_t t; 
        int16_t ax;
        int16_t ay;
        int16_t az;
        int16_t mx;
        int16_t my;
        int16_t mz;
    }  __attribute__((packed)) imuImpact;
    imuImpact   impact;
    LSM303DLH   Accelerometer;
public:
    IMUSensor(PinName sda, PinName scl, uint32_t idle = 1000, uint32_t stackSz = IMU_THREAD_STACK_SIZE, unsigned char* sp = NULL);
    virtual void InitResultsStatic(void);
    virtual void Loop(void);
    virtual void StoreLastImpact(void);
};

#endif // IMU_SENSOR_H
