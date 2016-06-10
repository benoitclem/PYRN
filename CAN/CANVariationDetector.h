#ifndef CAN_VARIATION_DETECTOR_H
#define CAN_VARIATION_DETECTOR_H


#include "CANInterface.h"
#include "MyCallBack.h"

#define NUM_FRAME_ENTRY_MAX 30

class CANVariationDetector: public MyCallBack {
private:
	typedef struct CANFrameEntry_ {
		uint16_t id;
		char data[8];
	} CANFrameEntry;
	CANFrameEntry fe[NUM_FRAME_ENTRY_MAX];
	uint16_t nFrameEntry;
	CANInterface *canItf;
public:
	CANVariationDetector(CANInterface *itf, int bus);
	virtual void event(int ID, void *data);
};


#endif