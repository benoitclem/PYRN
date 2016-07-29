
#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "L3GD20H.cpp"
#endif
#include "MyDebug.h"

#include "L3GD20H.h"

L3GD20H::L3GD20H(I2C *i2cDevice) {
    dev = i2cDevice;
    uint8_t d[16];
    // Check accelerometer presence
    this->devRead(L3GD20H_WHO_AM_I,d,1);
    if(d[0] == L3GD20H_ID) {
        INFO("L3GD20H(0x%02X) well detected!!",d[0]);
        // Config accelerometer
        this->basicConfig();
    } else
        ERR("Unknow ID 0x%02X ...",d[0]);
}

uint8_t L3GD20H::devRead(const uint8_t reg, uint8_t *data, uint8_t size) {
    uint8_t *pReg = (uint8_t*)&reg; 
    *pReg |= 0x80;
    INFO(">> %02x",L3GD20H_ADDR_W);
    __disable_irq();
    dev->write(L3GD20H_ADDR_W,(char*)pReg,1);
    dev->read(L3GD20H_ADDR_R,(char*)data,size);
    __enable_irq();
    return size;
}

uint8_t L3GD20H::devReadSingle(const uint8_t reg, uint8_t byte) {
    return this->devRead(reg,&byte,1);
}

uint8_t L3GD20H::devWrite(const uint8_t reg, uint8_t *data, uint8_t size) {
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
        dev->write(L3GD20H_ADDR_W,(char*)dataBuff,size+1);
        return size;
    }
}

uint8_t L3GD20H::devWriteSingle(const uint8_t reg, uint8_t byte) {
    return this->devWrite(reg,&byte,1);
}

// Hi Level API

void L3GD20H::basicConfig() {
    // Gyro

    // Low_ODR = 0 (low speed ODR disabled)
    this->devWriteSingle(L3GD20H_LOW_ODR, 0x00);

    // CTRL4
    //  0x00 
    //  (245 dps and default configs)
    gyrLSB = 3.738;  // mdps
    this->devWriteSingle(L3GD20H_CTRL4, 0x00);

    // CTRL1 
    //  0x6f
    //  DR = 01 (200 Hz ODR); 
    //  BW = 10 (50 Hz bandwidth); 
    //  PD = 1 (normal mode); 
    //  Zen = Yen = Xen = 1 (all axes enabled)
    this->devWriteSingle(L3GD20H_CTRL1, 0x6F);
    DBG("L3GD20H configuration done\r\n");
}

void L3GD20H::readRawGyr(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t rawData[6] = {0,0,0,0,0,0};
    if(this->devRead(L3GD20H_OUT_X_L,rawData,6)){
        *x = (int16_t)(rawData[0] | (rawData[1] << 8));
        *y = (int16_t)(rawData[2] | (rawData[3] << 8));
        *z = (int16_t)(rawData[4] | (rawData[5] << 8));
    }
}

void L3GD20H::readGyr(float *x, float *y, float *z) {
    int16_t rawX;
    int16_t rawY;
    int16_t rawZ;
    this->readRawGyr(&rawX,&rawY,&rawZ);
    *x = (((float) rawX) * gyrLSB) /1000.0;
    *y = (((float) rawY) * gyrLSB) /1000.0;
    *z = (((float) rawZ) * gyrLSB) /1000.0;
}