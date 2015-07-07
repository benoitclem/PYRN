#ifndef __LSM303DLH_H
#define __LSM303DLH_H

#include "mbed.h"

// CONSTS

#define WRITE_BUFF_MAX  16
#define LSM303DH_ID     0x49

// I2C Addresses (SAO -> 1)

#define LSM303D_ADDR_R   0x3B
#define LSM303D_ADDR_W  0x3A       

// REGISTERS

#define CTRL0       0x1F
#define CTRL1       0x20
#define CTRL2       0x21
#define CTRL3       0x22
#define CTRL4       0x23
#define CTRL5       0x24
#define CTRL6       0x25
#define CTRL7       0x26

#define WHO_AM_I    0x0F

#define TEMP_OUT_L  0x05
#define TEMP_OUT_H  0x06

#define STATUS_M    0x07
#define OUT_X_L_M   0x08
#define OUT_X_H_M   0x09
#define OUT_Y_L_M   0x0A
#define OUT_Y_H_M   0x0B
#define OUT_Z_L_M   0x0C
#define OUT_Z_H_M   0x0D

#define STATUS_A    0x27
#define OUT_X_L_A   0x28
#define OUT_X_H_A   0x29
#define OUT_Y_L_A   0x2A
#define OUT_Y_H_A   0x2B
#define OUT_Z_L_A   0x2C
#define OUT_Z_H_A   0x2D

class LSM303DLH {
private:
    float accLSB;
    float magLSB;
    
    I2C dev;
    uint8_t dataBuff[WRITE_BUFF_MAX];           // Who read more than WRITE_BUFF_MAX(16)?
    
    uint8_t devRead(const uint8_t reg, uint8_t *data, uint8_t size);
    uint8_t devReadSingle(const uint8_t reg, uint8_t byte);
    uint8_t devWrite(const uint8_t reg, uint8_t *data, uint8_t size);
    uint8_t devWriteSingle(const uint8_t reg, uint8_t byte);
public:
    LSM303DLH(PinName sda = p28, PinName scl = p27); 
    void basicConfig(void);
    void readRawAcc(int16_t *x, int16_t *y, int16_t *z);
    void readRawMag(int16_t *x, int16_t *y, int16_t *z);
    void readAcc(float *x, float *y, float *z);
    void readMag(float *x, float *y, float *z);
};

#endif