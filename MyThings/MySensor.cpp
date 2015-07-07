

#include "MySensor.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "MySensor.cpp"
#endif
#include "MyDebug.h"

MySensor::MySensor(const char* sName, uint8_t type, uint32_t idle, uint32_t sz, unsigned char *sp):
    MyThread(sName,sz,sp){
    // Each Sensor type have its ways to initialize the results structure
    // After this constructor don't forget to call InitResults(), Becarefull the
    // InitResultsStatic must be defined if you call InitResults with sz = -1
    sensorType = type;
    idleTime = idle;
}


MySensor::~MySensor() {
}

const char *MySensor::GetSensorName() {
    return tName;
}

uint8_t MySensor::GetSensorType() {
    return sensorType;
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
	sensors = (MySensor**) malloc (sizeof(MySensor*)*maxSensors);
	max = maxSensors;
	num = 0;
	dynamic = dynamic;
}

uint8_t MySensors::GetNSensors(void) {
	return num;
}

bool MySensors::AddSensor(MySensor *sensor) {	
	if((num+1)<max){
		*(sensors+num) = sensor;
		num++;
		return true;
	}
	return false;
}

MySensor* MySensors::GetSensor(uint8_t index) {
	if(index<num){
		return *(sensors+index);
	}
	return NULL;
}

MySensor* MySensors::PopLastSensor(void) {
	DBG("NUM = %d",num);
	if(num){
		DBG("Poping last sensor from %d",num);
		MySensor *s = *(sensors+(num-1));
		num--;
		return s;
	}
	return NULL;
}
