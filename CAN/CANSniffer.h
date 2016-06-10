
#ifndef CAN_SNIFFER_H
#define CAN_SNIFFER_H

#include "CANInterface.h"
#include "MyCallBack.h"

class CANSniffer: public MyCallBack {
protected:
    CANInterface *canItf;
    Queue<uint8_t, 1> Signal;
    int waitingId;
    uint8_t *dummy;
public:
    CANSniffer(CANInterface *itf, int32_t *lId = NULL, uint16_t len = 0);
    virtual bool wait(uint32_t id, int32_t timeout);
    virtual void event(int ID, void *data);
};

#endif // CAN_SNIFFER_H
