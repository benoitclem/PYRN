
#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "LSM303DLH.cpp"
#endif
#include "MyDebug.h"

#include "LSM303DLH.h"

LSM303DLH::LSM303DLH(PinName sda, PinName scl): dev(sda,scl) {
    uint8_t d[16];
    INFO("LSM303DLH Init (400kHz)");
    dev.frequency(400000);
    // Check accelerometer presence
    this->devRead(WHO_AM_I,d,1);
    if(d[0] == LSM303DH_ID) {
        INFO("LSM303DH(0x%02X) well detected!!",d[0]);
        // Config accelerometer
        this->basicConfig();
    } else
        ERR("Unknow ID 0x%02X ...",d[0]);
}

uint8_t LSM303DLH::devRead(const uint8_t reg, uint8_t *data, uint8_t size) {
    uint8_t *pReg = (uint8_t*)&reg; 
    *pReg |= 0x80;
    __disable_irq();
    dev.write(LSM303D_ADDR_W,(char*)pReg,1);
    dev.read(LSM303D_ADDR_R,(char*)data,size);
    __enable_irq();
    return size;
}

uint8_t LSM303DLH::devReadSingle(const uint8_t reg, uint8_t byte) {
    return this->devRead(reg,&byte,1);
}

uint8_t LSM303DLH::devWrite(const uint8_t reg, uint8_t *data, uint8_t size) {
    // Check size
    if((size+1)>WRITE_BUFF_MAX) {
        ERR("BAD SIZE WRITE");
        return 0;
    } else {
        // Put the reg addr in first byte place
        dataBuff[0] = reg | 0x80;
        for(int i = 0; i< size; i++)
            dataBuff[i+1] = data[i];
        // Do the Writing
        dev.write(LSM303D_ADDR_W,(char*)dataBuff,size+1);
        return size;
    }
}

uint8_t LSM303DLH::devWriteSingle(const uint8_t reg, uint8_t byte) {
    return this->devWrite(reg,&byte,1);
}

// Hi Level API

void LSM303DLH::basicConfig() {
    // Accelerometer
    // 0x00 = 0b00000000
    // AFS = 0 (+/- 2 g full scale) - > 0.061 g/LSB
    this->devWriteSingle(CTRL2, 0x00);
    accLSB = 61.0;
    // 0x57 = 0b01010111
    // AODR = 0101 (50 Hz ODR); AZEN = AYEN = AXEN = 1 (all axes enabled)
    this->devWriteSingle(CTRL1, 0x57);

    // Magnetometer
    // 0x64 = 0b01100100
    // M_RES = 11 (high resolution mode); M_ODR = 001 (6.25 Hz ODR)
    this->devWriteSingle(CTRL5, 0x64);
    // 0x20 = 0b00100000
    // MFS = 01 (+/- 4 gauss full scale)
    this->devWriteSingle(CTRL6, 0x20);
    magLSB = 80.0;
    // 0x00 = 0b00000000
    // MLP = 0 (low power mode off); MD = 00 (continuous-conversion mode)
    this->devWriteSingle(CTRL7, 0x00);
    
    DBG("LSM303DH configuration done\r\n");
}

void LSM303DLH::readRawAcc(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t rawData[6] = {0,0,0,0,0,0};
    if(this->devRead(OUT_X_L_A,rawData,6)){
        *x = (int16_t)(rawData[0] | (rawData[1] << 8));
        *y = (int16_t)(rawData[2] | (rawData[3] << 8));
        *z = (int16_t)(rawData[4] | (rawData[5] << 8));
    }
}

void LSM303DLH::readAcc(float *x, float *y, float *z) {
    int16_t rawX;
    int16_t rawY;
    int16_t rawZ;
    this->readRawAcc(&rawX,&rawY,&rawZ);
    *x = (((float) rawX) * accLSB) /1000000.0;
    *y = (((float) rawY) * accLSB) /1000000.0;
    *z = (((float) rawZ) * accLSB) /1000000.0;
}


void LSM303DLH::readRawMag(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t rawData[6] = {0,0,0,0,0,0};
    if(this->devRead(OUT_X_L_M,rawData,6)){
        *x = (int16_t)(rawData[0] | (rawData[1] << 8));
        *y = (int16_t)(rawData[2] | (rawData[3] << 8));
        *z = (int16_t)(rawData[4] | (rawData[5] << 8));
    }
}

void LSM303DLH::readMag(float *x, float *y, float *z) {
    int16_t rawX;
    int16_t rawY;
    int16_t rawZ;
    this->readRawMag(&rawX,&rawY,&rawZ);
    *x = (((float) rawX) * magLSB) / 1000000.0;
    *y = (((float) rawY) * magLSB) / 1000000.0;
    *z = (((float) rawZ) * magLSB) / 1000000.0;
}