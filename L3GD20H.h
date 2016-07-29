#ifndef __L3GD20H_H
#define __L3GD20H_H

#include "mbed.h"

// CONSTS

#define WRITE_BUFF_MAX  16
#define L3GD20H_ID     0x49

// I2C Addresses (SAO -> 1)

#define L3GD20H_ADDR_R  0xD7
#define L3GD20H_ADDR_W  0xD6       

// REGISTERS

#define L3GD20H_WHO_AM_I    0x0F
#define L3GD20H_CTRL1       0x20
#define L3GD20H_CTRL2       0x21
#define L3GD20H_CTRL3       0x22
#define L3GD20H_CTRL4       0x23 
#define L3GD20H_CTRL5       0x24
#define L3GD20H_REFERENCE   0x25
#define L3GD20H_OUT_TEMP    0x26
#define L3GD20H_STATUS      0x27
#define L3GD20H_OUT_X_L     0x28
#define L3GD20H_OUT_X_H     0x29
#define L3GD20H_OUT_Y_L     0x2A 
#define L3GD20H_OUT_Y_H     0x2B
#define L3GD20H_OUT_Z_L     0x2C
#define L3GD20H_OUT_Z_H     0x2D 
#define L3GD20H_FIFO_CTRL   0x2E
#define L3GD20H_FIFO_SRC    0x2F
#define L3GD20H_IG_CFG      0x30
#define L3GD20H_IG_SRC      0x31
#define L3GD20H_IG_THS_XH   0x32 
#define L3GD20H_IG_THS_XL   0x33
#define L3GD20H_IG_THS_YH   0x34
#define L3GD20H_IG_THS_YL   0x35
#define L3GD20H_IG_THS_ZH   0x36
#define L3GD20H_IG_THS_ZL   0X37
#define L3GD20H_IG_DURATION 0x38
#define L3GD20H_LOW_ODR     0x39

class L3GD20H {
private:
    float gyrLSB;
    
    I2C *dev;
    uint8_t dataBuff[WRITE_BUFF_MAX];           // Who read more than WRITE_BUFF_MAX(16)?
    
    uint8_t devRead(const uint8_t reg, uint8_t *data, uint8_t size);
    uint8_t devReadSingle(const uint8_t reg, uint8_t byte);
    uint8_t devWrite(const uint8_t reg, uint8_t *data, uint8_t size);
    uint8_t devWriteSingle(const uint8_t reg, uint8_t byte);
public:
    L3GD20H(I2C *i2cDevice); 
    void basicConfig(void);
    void readRawGyr(int16_t *x, int16_t *y, int16_t *z);
    void readGyr(float *x, float *y, float *z);
};

#endif