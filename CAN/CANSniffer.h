
#ifndef CAN_SNIFFER_H
#define CAN_SNIFFER_H

#include "CANInterface.h"
#include "MyCallBack.h"

class CANSniffer: public MyCallBack {
protected:
    CANInterface *canItf;
public:
    CANSniffer(CANInterface *itf, int32_t *lId = NULL, uint16_t len = 0);
    virtual void event(int ID, void *data);
};

#endif // CAN_SNIFFER_H
