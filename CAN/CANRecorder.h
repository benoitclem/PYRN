#ifndef CAN_RECORDER_H
#define CAN_RECORDER_H

#include "CANRecorderCalculator.h"
#include "MySensor.h"
#include "CANInterface.h"
#include "MyCallBack.h"

class CANRecorder: public MyCallBack, public MySensorBase {
protected:
	Mutex BuffMtx;
	typedef struct _learningStuffs{
		bool isLearning;
		int nTicks;
		int *tmp;
		uint16_t oldTick;
	} learningStuffs;
    CANInterface *canItf;
    CANRecorderCalculator *calc;
    bool activated;    
    uint16_t offset;
    int nPoints;
    int nPointsMax;
    char *dataResult;
    learningStuffs lStruct;
    uint16_t *dt;
    uint16_t *nSamples;
    char ttt[24];
public:
	CANRecorder(CANInterface *itf, CANRecorderCalculator *calculator);
	virtual ~CANRecorder();
	virtual bool getActivated(void) { return activated; };
	virtual int getNPoints(void) { return nPoints; };
	virtual int getNPointsMax(void) { return nPointsMax; };
	virtual CANRecorderCalculator *GetCalculator() { return calc; };
	virtual void Activate() { activated = true;};
	virtual void DeActivate() { activated = false;};
	virtual void SetupCalcFilter();
	virtual void Capture(char *data, uint16_t *len);
	virtual void event(int ID, void *data);
};

#endif // CAN_RECORDER_H
