
#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "main.cpp"
#endif
#include "MyDebug.h"

#include "mbed.h"
#include "main.h"

#include "rtos.h"

#include "MyOsHelpers.h"
#include "MyWatchdog.h"
#include "MyThread.h"
#include "MyBlinker.h"
#include "MyMemoryAllocator.h"

#include "IMUSensor.h"
#include "GPSSensor.h"

#include "CANInterface.h"
#include "CANSniffer.h"
#include "CANCommunicator6A.h"
#include "CANDiagSensor.h"

#include "PyrnUSBModem.h"
#include "HTTPClient.h"
#include "TCPSocketConnection.h"

#include "NTPClient.h"
#include "ComHandler.h"

#include "Storage.h"

#define BLINKER
#define THREAD_MONITOR
//#define APP_TEST
#define APP_SHARKAN
//#define APP_SDSPI
//#define APP_CANONAIR
#define CAN_SIMULATOR

bool printThread;

char dataResult[1024] /* __attribute((section("AHBSRAM0")))*/;
extern MyMemoryAllocator memAlloc;
Storage *storage;
CANInterface *c;

void threadMonitor(void const *args) {
    while (true) {
    	if(printThread)
        	PrintActiveThreads();
        Thread::wait(500);
    }
}

MainClass::MainClass(uint8_t nStaticts, uint8_t  nDynamics):
 staticSensors(nStaticts, true),
 dynamicSensors(nDynamics){

}


void MainClass::CheckSDFileSystem(void) {
	uint8_t wbuff[8] = {'a','b',0x05,'d','e','f','g','h'};
	uint16_t sWrite = storage->WriteData("Test",(uint8_t*)wbuff, 8, false);
	uint16_t sRead = storage->ReadData("Test",(uint8_t*)dataResult, 1024, 0, false);
	DBG_MEMDUMP("T",dataResult,sRead);
	storage->daRemove("/sd/Test.txt");
	sWrite = storage->WriteData("impacts",(uint8_t*)wbuff, 8, false);
	sRead = storage->ReadData("impacts",(uint8_t*)dataResult, 1024,0,false);
	DBG_MEMDUMP("ReadData",dataResult,sRead);
	//storage->daRemove("/sd/impacts.txt");
}


void MainClass::LoadCalculators(void) {
	uint16_t l = storage->ReadDataSize("calcs");
	if(l) {
		l = storage->ReadData("calcs",(uint8_t*)dataResult,1024,0,false);
		if(l) {
			DBG("There is some calculator data stored recall them");
			DBG("Storage Calculators",dataResult,l);
			this->event(l,dataResult);
		}
	} else {
		DBG("No Calculator Data to read");
	}
}


void MainClass::run(void) {

	//char *dataResult = (char*) memAlloc.malloc(sizeof(char)*1024);

#ifdef BLINKER
	unsigned char *stack1 = (unsigned char*) memAlloc.malloc(BLINKER_THREAD_STACK_SIZE);
	unsigned char *stack2 = (unsigned char*) memAlloc.malloc(BLINKER_THREAD_STACK_SIZE);

	// Light 
	PwmOut led_red(LED1);
	PwmOut led_green(LED2);

	MyBlinker br = MyBlinker(&led_red,1000,1.0,0.0,BLINKER_THREAD_STACK_SIZE,stack1);
	MyBlinker bg = MyBlinker(&led_green,2000,1.0,0.0,BLINKER_THREAD_STACK_SIZE,stack2);
#endif

#ifdef THREAD_MONITOR
	Thread monitor(threadMonitor,NULL,osPriorityNormal,512);
#endif

#if defined APP_TEST
	DBG("TEST APP");
	
	uint16_t i = 0;
	
	while(1) {
		DBG("TEST LOOP");
		i++;
		Thread::wait(250);
		if(i == 20){
			DBG("TEST STOP");
			br.Stop();
			DBG("TEST WAIT");
			br.WaitEnd();
		}	
	}

	while(1) {
		DBG("TEST APP");
		Thread::wait(1000);
	}
	
#elif defined APP_SDSPI
	DBG("SPI SD APP");
	/*
	SDFileSystem sd(p5,p6,p7,p8,"sd");
	char buff[256];
	while(1) {
		FILE *fp = fopen("/sd/myfile.txt", "r");
		fread(buff,1,256,fp);
		DBG("buff %s\r\n", buff);
		fclose(fp);
		Thread::wait(1000);
	}
	*/

	Timer t;
	
	//storage = new Storage(4);

	storage->daRemove("/sd/Test.txt");
	char wbuff[10];
	char rbuff[1024];
	strcpy(wbuff,"hellowww\n");
	for(uint8_t i = 0; i< 20; i++) {
		t.start();
		uint16_t sWrite;
		for(uint8_t j = 0; j<10; j++)
			sWrite = storage->WriteData("Test",(uint8_t*)wbuff, 9, false);
		t.stop();
		DBG("Storage Write [%f] (%d/9)\n",t.read(),sWrite);
		t.reset();
		uint16_t sTotal = storage->ReadDataSize("Test");
		t.start();
		uint16_t sRead = storage->ReadData("Test",(uint8_t*)rbuff, 1024, 0, false);
		t.stop();
		DBG("Storage Read [%f] (%d/%d)\n",t.read(),sRead,sTotal);
		t.reset();
		DBG_MEMDUMP("Storage",rbuff,sRead);
	}
	delete(storage);

	while(1){
		Thread::wait(1000);	
	}
	
#elif defined APP_SHARKAN
	DBG("SHARKAN APP");
	
	//printThread = true;

	CANInterface canItf;
	c = &canItf;
	CANSniffer canSnif(&canItf);

	canItf.Start();
	canItf.Run();

	while(1){
		Thread::wait(1000);	
	}

	canItf.Stop();

#elif defined APP_CANONAIR
	DBG("CAN APP");
	DBG("SIZE %d", sizeof(FATFS));
	DBG("SIZE %d", sizeof(FIL));
	DBG("SIZE %d", sizeof(FATFS_DIR));
	DBG("SIZE %d", sizeof(FILINFO));
	DBG("SIZE %d", sizeof(FRESULT));
	
	DBG("SIZE %d", sizeof(FATFileSystem));
	//storage = new Storage(4);
	
	printThread = true;
	// Set a wdt with 60s timer
	MyWatchdog wdt(60);

	//CheckSDFileSystem();
	PyrnUSBModem modem;
	
	int connected = modem.connect("a2bouygtel.com","","");
	//int connected = 0;
	if(connected != 0) {
		DBG("Could not connect modem.. Now reset system ");
		NVIC_SystemReset();
	} else {
	
		wdt.Feed();
		
		/*
		int r = storage->ReadData("impacts",(uint8_t*)dataResult,1024,0,false);
		DBG_MEMDUMP("impacts data",dataResult,r);
		*/
	
		// NTPClient
		NTPClient ntp;
		ntp.setTime("0.europe.pool.ntp.org");

		// 3G (configure the system to return ASAP for the first connection)
		ComHandler comm(this,"AAAAAAAAAAAAAAA",ComHandler::TT_ASAP);

		//ComHandler comm(this,"AAAAAAAAAAAAAAA");

		// CAN 
		CANInterface canItf;
		c = &canItf;
		//CANSniffer canSnif(&canItf);

		// The static list of sensors (GPS/IMU/...)
		IMUSensor *imu = new IMUSensor(p28,p27);
		staticSensors.AddSensor(imu);
		//GPSSensor gps = GPSSensor gps(p13,p14,4,250);
		//staticSensors.AddSensor(&gps);

		// Request the first calculators
#ifdef CAN_SIMULATOR
		// Diags
		CANDiagCalculatorHeader SimuHdr;
		SimuHdr.hdrVer = 1;
		SimuHdr.diagCode = 0x6A;
		SimuHdr.speed = 500;
		SimuHdr.addrSrc = 0x752;
		SimuHdr.addrDst = 0x652;
		SimuHdr.pinh = 6;
		SimuHdr.pinl = 14;
		SimuHdr.cmdLen = 0;
		char *SimuData = (char*) &SimuHdr;
				
		// Simu Calculator
		CANDiagCalculator *simuCalc = new CANDiagCalculator((const char*)SimuData);
		
		// Comm handlers
		uint8_t bus = canItf.BusFromPins(simuCalc->GetPinH(),simuCalc->GetPinL());
		CANCommunicatorSim6A *Sim6A = new CANCommunicatorSim6A(&canItf,bus,simuCalc);
		Sim6A->Start();
#endif

		// Threads
		imu->Start();
		//gps.Start();
		comm.Start();
		//DiagSensor6A->Start();
		canItf.Start();


		imu->Run();
		//gps.Run();
		//comm.Run();
		//DiagSensor6A->Run();
		canItf.Run();
		
		Thread::wait(2000);

		uint16_t len = 0;
		
		//LoadCalculators();
				
		while(1){
			MySensor *s;
			// Request StaticSensors
			wdt.Feed();
			DBG("MainThread [STA:%d/DYN:%d]",staticSensors.GetNSensors(),dynamicSensors.GetNSensors());
			for(int i = 0; i<staticSensors.GetNSensors(); i++) {
				DBG("Capture STA SENSOR[%d]",i+1);
				s = staticSensors.GetSensor(i);
				if(s != NULL) {
					s->Capture(dataResult,&len);
					if(len) {
						uint8_t st = s->GetSensorType();
						time_t seconds = time(NULL);
						DBG("(%04d) %s(%d) Got %03d chars",seconds,s->GetSensorName(),st,len);
						if(comm.AddResults(st,dataResult,len)){
							DBG("Result have been writen");

 						} else {
							DBG("Result could not be writen");
						}
					}
				}
			}
			if(dynSensorAccess.trylock()) {
				// Request the dynamicSensors 
				for(int i = 0; i<dynamicSensors.GetNSensors(); i++) {
					DBG("Capture DYN SENSOR[%d]",i+1);
					s = dynamicSensors.GetSensor(i);
					if(s != NULL) {
						s->Capture(dataResult,&len);
						if(len) {
							uint8_t st = s->GetSensorType();
							time_t seconds = time(NULL);
							DBG("(%04d) %s(%d) Got %03d chars",seconds,s->GetSensorName(),st,len);
							if(comm.AddResults(st,dataResult,len)){
								DBG("Result have been writen");
							} else {
								DBG("Result could not be writen");
							}
						}
					}
				}
				dynSensorAccess.unlock();
			}
		    Thread::wait(1000);
		}
		
		//Sim6A.Stop();

		//imu->Stop();
		//gps.Stop();
		//DiagSensor6A.Stop();   
		canItf.Stop();
	}
#endif

#ifdef BLINKER
	br.Stop();
	bg.Stop();
#endif

}

void MainClass::event(int ID, void *data) {
	DBG("event received");
#if defined APP_CANONAIR
	osStatus s = dynSensorAccess.lock(4000);
	if(s == osOK){
		// Delete the sensors for dybamic pool
		DBG("There is a configuration pending stop sensors");
		while(1) {
			CANDiagSensor6A *s = (CANDiagSensor6A*) dynamicSensors.PopLastSensor();
			if(s != NULL) {
				DBG("%s Stop+WaitEnd",s->GetSensorName());
				s->Stop();
				DBG("Got Stopped");
				s->WaitEnd();
				DBG("Ended");
			} else
				break;
			CANDiagCalculator *calc = s->GetDiagCalculator();
			CANCommunicator6A *com = s->GetCommunicator();
			DBG("Release Sensor");
			delete(s);
			DBG("Release ComHandler");
			delete(com);
			DBG("Release DiagCalculator");
			delete(calc);
			memAlloc.PrintResume(false);
		DBG("Dynamic list is empty");
		}
		uint8_t nCalc = 0;
		char *pData = (char*) data;
		int offset = 0;
		int inc = 0;
		int dataSz = ID;
		DBG_MEMDUMP("RX DATA", pData, dataSz);
			CANDiagCalculator *DiagCalc = new CANDiagCalculator();
		while(1) {
			inc = DiagCalc->SetData((const char*)pData+offset);
			DBG_MEMDUMP("CALC DATA", DiagCalc->GetDataPointer(), sizeof(CANDiagCalculatorHeader));
			if(inc) {
				DBG("Incrementing pointer offset to %d",inc);
				offset += inc;
			} else {
				DBG("Calculator data are corrupted ... free memory and abort");
				delete(DiagCalc);
				break;
			}
			if(DiagCalc->Ready()) {
				uint8_t dcode = DiagCalc->GetDiagCode();
				if(dcode == 0x6A) {
					uint8_t ch = DiagCalc->GetPinH();
					uint8_t cl = DiagCalc->GetPinL();
					int bus = c->BusFromPins(ch,cl);
					DBG("Calculator is ready [%d/%d-%d]",ch,cl,bus);
					if(bus) {
							CANCommunicator6A *Diag6A = new CANCommunicator6A(c,bus,DiagCalc);
							CANDiagSensor6A *DiagSensor6A = new CANDiagSensor6A(DiagCalc,Diag6A);
							DiagSensor6A->Start();
							DiagSensor6A->Run();
							DBG("Adding Sensor %p to dynamic list",DiagSensor6A);
							dynamicSensors.AddSensor(DiagSensor6A);
					} else {
						DBG("Wrong BUS number (%d-%d-%d)... free memory and abort",ch,cl,bus);
						delete(DiagCalc);
						break;
					}
				} else {
					DBG("Unknown Calculator DiagCode(0x%02X)",dcode);
					delete(DiagCalc);
				}
			} else {
				DBG("Calculator is not ready, something got wrong  ... free memory and abort");
				delete(DiagCalc);
				break;
			}
			// Check for ending
			if(offset>=dataSz) {
				DBG("Successfully instanciate %d sensors",nCalc+1);
				break;
			} else {
				nCalc++;
			}
		}
		
		dynSensorAccess.unlock();
		//
	} else if(s == osEventTimeout) {
		DBG("Could not obtain lock to change the ongoing Diagnoses.. data will be dropped");
	}
#endif
}

int main(void) {
	set_time(0);
    debug_init();
    debug_set_newline("\r\n");
    debug_set_speed(115200);
    
    printThread = false;
    
    // Main Function
    MainClass m(2,5);
    m.run();
    
    // Dead end
    while(1) Thread::wait(1000);
}

