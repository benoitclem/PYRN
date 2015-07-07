#include "CANFifoMessage.h"

CANFifoMessage::CANFifoMessage() {
    this->head = 0;
    this->tail = 0;
}
uint32_t CANFifoMessage::available() {
    return (FIFO_SIZE + this->head - this->tail) % FIFO_SIZE;
}

uint32_t CANFifoMessage::free() {
    return (FIFO_SIZE - 1 - available());
}

bool CANFifoMessage::put(CANMessage data){
    uint32_t next;

    // check if FIFO has room
    next = (this->head + 1) % FIFO_SIZE;
    if (next == this->tail) {
        return false;   // FIFO full
    }

    this->buffer[this->head] = data;
    this->head = next;

    return true;
}

bool CANFifoMessage::get(CANMessage* data) {
    uint32_t next;

    // check if FIFO has data
    if (this->head == this->tail) {
        return false; // FIFO empty
    }

    next = (this->tail + 1) % FIFO_SIZE;

    *data = this->buffer[this->tail];
    this->tail = next;

    return true;
}
