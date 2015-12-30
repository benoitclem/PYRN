

#ifndef CAN_CORRELATOR_H
#define CAN_CORRELATOR_H


#include "CANInterface.h"
#include "MyCallBack.h"
#include "CANVariableData.h"

#define CAN_ID_PROMISCUOUS_MODE		0xffffffff
#define CAN_CORRELATOR_MAX_FRAMES	10
#define CAN_FRAME_MAX_LEN			64
#define CAN_MAX_FRAMES				40
#define CAN_MAX_VARS				40

typedef struct _frameEntry{
	int  id;						// The calculator ID
	char len;						// The Len of CAN Frame
	char data[CAN_FRAME_MAX_LEN];	// The CAN Frame
} frameEntry;

typedef struct _variableEntry {
	int id;							// The calculator ID
	int index;						// The index of data
	int len;						// The len of data
	CANVariableData *vData;			// Link to the real Data
} variableEntry;

class CANCorrelator: public MyCallBack {
private:
	CANInterface *canItf;
	int bus;
	int curFrames;
	int maxFrames;
	frameEntry frameList[CAN_MAX_FRAMES];
	int curVariables;
	int maxVariables;
	variableEntry variableList[CAN_MAX_VARS];
public:
	CANCorrelator(CANInterface *itf, int bus);
	virtual float BasicCorrelation(char len, char *dataOne, char *dataTwo);
	virtual int	SearchSimilarFrame(int id, char len, char *data);
	virtual int SearchSimilarVariable(int id, int index, int len);
	virtual int InsertNewVariable(int id, int s, int c);
	virtual void TrackVariations(int id, char len, char *dataOne, char *dataTwo);
	virtual void PrintVariables();
	virtual void event(int ID, void *data);
};

#endif // CAN_CORRELATOR_H