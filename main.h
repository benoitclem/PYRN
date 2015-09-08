
#ifndef MAIN_H
#define MAIN_H


#include "MyCallBack.h"
#include "MySensor.h"

class MainClass: MyCallBack{
protected:
	Mutex dynSensorAccess;
	MySensors staticSensors;
	MySensors dynamicSensors;
public:
	MainClass(uint8_t nStaticts, uint8_t  nDynamics);
	void CheckSDFileSystem(void);
	void LoadCalculators(void);
	void run(void);
	virtual void event(int ID, void *data);
};

#endif /*MAIN_H*/
