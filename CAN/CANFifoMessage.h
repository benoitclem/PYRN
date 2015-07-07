#ifndef CAN_FIFO_MESSAGE
#define CAN_FIFO_MESSAGE

#include "mbed.h"
#include "CAN.h"

#define FIFO_SIZE   25

class CANFifoMessage {
    CANMessage buffer[FIFO_SIZE];
    uint32_t head, tail;
public:
    CANFifoMessage();
    bool put(CANMessage data);// returns 0 on success
    bool get(CANMessage* data);
    uint32_t available();
    uint32_t free();
};

#endif /* CAN_FIFO_MESSAGE */
