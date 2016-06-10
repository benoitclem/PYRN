#ifndef GPS_SENSOR_UBX_H
#define GPS_SENSOR_UBX_H

#include "mbed.h"
#include "GPSSensor.h"
#include "Configs.h"

#include "ubxMessagesDefs.h"

class GPSSensorUBX: public GPSSensor {
public:
    GPSSensorUBX(PinName tx, PinName rx, uint32_t idle = 1000, uint32_t stackSz = GPS_THREAD_STACK_SIZE, unsigned char* sp = NULL);
	virtual ~GPSSensorUBX();

    void ComputeCheckSum(unsigned char* buffer, int size, unsigned char* checkSumA, unsigned char* checkSumB);
    void AppendCheckSum(UBXMsg *msg);

    // Sensor stuffs
    //virtual void Main(void) {};
    virtual bool NeedImpact(void) { return true;};
    virtual void Sample(void);

    // Low Level GPS API
    virtual void SendFrame(char *f, int size);
	virtual int ReceiveFrame(uint32_t timeOutUs = 100000);
	
    // Framer GPS API
    virtual void FillHeader(UBXMsg *msg, UBXMessageClass msgClass, UBXMessageId msgId, int len);
	virtual void PollCmd(UBXMessageClass msgClass, UBXMessageId msgId);
	
    // Cmd GPS API
    virtual void MON_VER(void);
	virtual void NAV_POSLLH(void);
    virtual void NAV_RATE(void);

    // High Levek GPS API
    virtual void DoSerialAGPS(void);
    virtual void DoNetworkAGPS(char *servName);
    
    virtual bool GetTime(uint32_t *date, uint32_t *time);
	virtual bool GetPosition(int32_t *lat, int32_t *lon, int32_t *alt, uint32_t *hAcc, uint32_t *vAcc);
    virtual bool GetDop();
};

#endif // GPS_SENSOR_UBX_H