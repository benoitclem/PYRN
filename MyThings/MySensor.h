
#ifndef MYSENSOR_H
#define MYSENSOR_H

#include "MyThread.h"
#include "rtos.h"

#define SENSOR_NAME_NONE		"SENSORNONE"
#define SENSOR_TYPE_NONE		0
#define SENSOR_NAME_IMU			"SENSORIMU"
#define SENSOR_TYPE_IMU			1
#define SENSOR_NAME_CAN_DIAG	"SENSORCANDIAG"
#define SENSOR_TYPE_CAN_DIAG	2
#define SENSOR_NAME_GPS     	"SENSORGPS"
#define SENSOR_TYPE_GPS     	3

#define SENSOR_RESSZ_DEFAULT -1

// Forward Declaration
class MySensor;

typedef struct _sensors {
	Mutex access;
	MySensor **sens;
	uint8_t num;
	uint8_t max;
} Sensors;

class MySensor:public MyThread {
protected:
    uint8_t sensorType;
    typedef struct _dataResult{
        uint8_t *start;
        uint8_t *current;
        uint16_t num;
        uint16_t max;
        bool dynamic;
    } dataResult;
    dataResult results;
    Mutex resultsMutex;
    uint32_t idleTime;
public:
    MySensor(const char* sName = SENSOR_NAME_NONE, uint8_t type = SENSOR_TYPE_NONE, uint32_t idle = 1000, uint32_t sz = DEFAULT_STACK_SIZE, unsigned char * sp = NULL);
    virtual ~MySensor();
    virtual const char *GetSensorName();
    virtual uint8_t GetSensorType();
    virtual void SetIdleTime(uint32_t it);
    virtual void Main();
    virtual void InitResults(int16_t size);
    virtual void ReleaseResults();
    virtual void InitResultsStatic() = 0;
    virtual void Loop() = 0;
    virtual void StoreLastImpact(void) = 0;
    virtual void Capture(char *data, uint16_t *len);
};

// This class is not thread safe, you need to check the acces by yourself

class MySensors {
protected:
	MySensor **sensors;
	uint8_t num;
	uint8_t max;
	bool    dyn;
public:
	MySensors(uint8_t maxSensors, bool dynamic = false);
	uint8_t GetNSensors(void);
	bool AddSensor(MySensor *sensor);
	MySensor* GetSensor(uint8_t index);
	MySensor* PopLastSensor(void);
};

#endif
