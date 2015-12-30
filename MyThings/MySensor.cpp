

#include "MySensor.h"
#include "Configs.h"

#define __DEBUG__ SENSOR_LIST_DBG_LVL
#ifndef __MODULE__
#define __MODULE__ "MySensor.cpp"
#endif
#include "MyDebug.h"

MySensorBase::MySensorBase(uint8_t type) {
    sensorType = type;
}

uint8_t MySensorBase::GetSensorType() {
    return sensorType;
}

MySensor::MySensor(const char* sName, uint8_t type, uint32_t idle, uint32_t sz, unsigned char *sp):
    MySensorBase(type),
    MyThread(sName,sz,sp){
    // Each Sensor type have its ways to initialize the results structure
    // After this constructor don't forget to call InitResults(), Becarefull the
    // InitResultsStatic must be defined if you call InitResults with sz = -1
    idleTime = idle;
}

MySensor::~MySensor() {
}

const char *MySensor::GetSensorName() {
    return tName;
}

void MySensor::SetIdleTime(uint32_t idle) {
    idleTime = idle;
}

void MySensor::Main() {
    while(running){
        Loop();
        StoreLastImpact();
        Thread::wait(idleTime);
    }
}

void MySensor::InitResults(int16_t size = -1) {
    if(size == -1) {
        InitResultsStatic();
    } else {
        results.dynamic = true;
        results.start = (uint8_t*) malloc (sizeof(uint8_t)*size);
        results.current = results.start;
        results.max = size;
    }
    results.num = 0;
}

void MySensor::ReleaseResults() {
    if(results.dynamic && (results.start != NULL)) {
        free(results.start);
        results.start = NULL;
        results.current = NULL;
        results.max = 0;
    }
}

// Here we could imgine a mecanism that allow passing smaller buffer that the results data count

void MySensor::Capture(char *data, uint16_t *len) {
    // Warning here, lock is not the lock we 
    if(results.start != NULL) {
        if(resultsMutex.trylock()){
            // Copy the results
            memcpy(data,results.start,results.num);
            *len = results.num;
            // Reset the results struct
            results.current = results.start;
            results.num = 0;
            resultsMutex.unlock();
            return;
        }
    }
    len = 0;
}


MySensors::MySensors(uint8_t maxSensors, bool dynamic) {
	sensors = (MySensorBase**) malloc (sizeof(MySensorBase*)*maxSensors);
	max = maxSensors;
	num = 0;
	dynamic = dynamic;
}

uint8_t MySensors::GetNSensors(void) {
	return num;
}

bool MySensors::AddSensor(MySensorBase *sensor) {
	if((num+1)<=max){
		*(sensors+num) = sensor;
		num++;
		return true;
	}
	return false;
}

MySensorBase* MySensors::GetSensor(uint8_t index) {
	if(index<num){
		return *(sensors+index);
	}
	return NULL;
}

MySensorBase* MySensors::PopLastSensor(void) {
	if(num){
		DBG("Poping last sensor from %d",num);
		MySensorBase *s = *(sensors+(num-1));
		num--;
		return s;
	}
	return NULL;
}
