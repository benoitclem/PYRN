
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

#include "storageBase.h"

#include "Configs.h"

#include "sd.h"

//#define BLINKER
#define THREAD_MONITOR
//#define APP_TEST
//#define APP_CANV10
#define CAN_SIMULATOR
//#define APP_SHARKAN
//#define APP_STORAGE
//#define APP_SDSPI
#define APP_CANONAIR
bool printThread;

bool newCalcPending;
char dataResult[1024]  __attribute((section("AHBSRAM1")));
extern MyMemoryAllocator memAlloc;

storage *calcStorage;

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

void MainClass::LoadCalculators(void) {
/*	int dataSz = 0;
	char *pData = dataResult;
	calcStorage.Read((char*)&dataSz,0,sizeof(int));
	DBG("Load new calculator Size = %d Data",dataSz)
	if((dataSz!=0)&& (dataSz<=1024)) {
		calcStorage.Read(pData,1,dataSz);
		event(dataSz,pData);
	}*/
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

#if defined APP_STORAGE
	int r = 0;
	int sz = 20;
	char* buff = (char*) malloc(sizeof(char)*256);
	DBG("STORAGE APP RAMSTORAGE - TEST");
	ramStorage *rs = new ramStorage(1024); 
	ramCircBuff *rcb = new ramCircBuff(1024); 

	DBG("SECTOR 0");
	sz = 10;
	r = rs->Read((char*)&sz,0,sizeof(int));
	DBG("[%d][0] -> sz = %d",r,sz);
	sz = 30;
	r = rs->Write((char*)&sz,0,sizeof(int));
	r = rs->Read((char*)&sz,0,sizeof(int));
	DBG("[%d][0] -> sz = %d",r,sz);

	DBG("SECTOR 1");
	memset(buff,0xa5,128);
	r= rs->Read(buff,1,128);
	DBG("[%d]",r);
	// Lit a vide donc que des zero si le buffer est clear a l'instanciation
	DBG_MEMDUMP("dbg",buff,128); 
	memset(buff,0xa5,128);
	r = rs->Write(buff,1,128);
	memset(buff,0xaa,128);
	r = rs->Read(buff,1,20);
	DBG("[%d]",r);
	// 20 octects a 0xa5 le reste a 0Xaa 
	DBG_MEMDUMP("dbg",buff,128);
	r = rs->Read(buff,1,128);
	DBG("[%d]",r);
	DBG_MEMDUMP("dbg",buff,128);

	memset(buff,0xa0,128);
	r = rs->Write(buff,1,20);
	memset(buff,0x0a,128);
	r = rs->Read(buff,1,64);
	// 20o 0xa0 puis jusqua 64 de 0xa5 le reste de 0x0a
	DBG_MEMDUMP("dbg",buff,128);

	DBG("SECTOR 3");
	DBG("[%d]",r);
	r = rs->Read(buff,3,64);
	DBG("[%d]",r);
	DBG_MEMDUMP("dbg",buff,128);
	r = rs->Write(buff,3,64);
	DBG("[%d]",r);
	DBG_MEMDUMP("dbg",buff,128);

	DBG("rcb");
	memset(buff,0x90,32);
	r = rcb->Get(buff,16);
	DBG("[%d]",r);
	if(r)
		DBG_MEMDUMP("dbg",buff,r);
	else
		DBG("GOT NO DATA");
	memset(buff,0x01,32);
	r = rcb->Put(buff,3);
	DBG("[%d]",r);
	memset(buff,0x02,32);
	r = rcb->Get(buff,256);
	DBG("[%d]",r);
	DBG_MEMDUMP("dbg",buff,r);

	DBG("4 PUTS");
	int i = 4;
	char c = 0;
	while(i--) {
		memset(buff,c++,141);
		r = rcb->Put(buff,141);
		DBG("%d - [%d]",i,r);
	}

	DBG("15 PUTS + GETS");
	i = 15;
	while(i--) {
		memset(buff,c++,141);
		r = rcb->Put(buff,141);
		DBG("%d - [%d]",i,r);
		r = rcb->Get(buff,256);
		DBG("%d - [%d]",i,r);
		DBG_MEMDUMP("dbg",buff,r);
	}

	DBG("15 PUTS");
	i = 15;
	while(i--) {
		memset(buff,c++,141);
		r = rcb->Put(buff,141);
		DBG("%d - [%d]",i,r);
	}

	DBG("15 GETS");
	i = 15;
	while(i--) {
		memset(buff,c++,256);
		r = rcb->Get(buff,256);
		DBG("%d - [%d]",i,r);
		DBG_MEMDUMP("dbg",buff,r);
	}

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

	calcStorage = new sdStorage(p5,p6,p7,p8,0,100);	 // use to store calcultors & for spi test
	sdCircBuff dataStorage(p5,p6,p7,p8,100,100); // use to store the results

	Timer t;
	/*
	
	memset(dataResult,0x00,1024);
	calcStorage.Write(dataResult,0,1024);
	
	memset(dataResult,0x01,1024);
	calcStorage.Write(dataResult,0,10,16);
	
	memset(dataResult,0x02,1024);
	calcStorage.Write(dataResult,0,3,3);
	
	memset(dataResult,0xA5,640);
	calcStorage.Write(dataResult,0,640,256);
	memset(dataResult,0xA0,1024);
	calcStorage.Read(dataResult,0,1024);
	DBG_MEMDUMP("SD DATA 1:", dataResult, 1024);
	
	memset(dataResult,0x00,1024);
	calcStorage.Read(dataResult,0,10,24);
	DBG_MEMDUMP("SD DATA small:", dataResult, 10);
	
	memset(dataResult,0x00,1024);
	calcStorage.Read(dataResult,0,3,2);
	DBG_MEMDUMP("SD DATA small:", dataResult, 3);
	
	calcStorage.Read(dataResult,0,1024,256);
	DBG_MEMDUMP("SD DATA 2:", dataResult, 1024);
	
	memset(dataResult,0xA0,1024);
	calcStorage.Write(dataResult,0,1024);
	memset(dataResult,0xA5,1024);
	calcStorage.Read(dataResult,0,640);
	DBG_MEMDUMP("SD DATA:", dataResult, 1024);
	*/
	int retries = 6;
	while(retries--) {
		memset(dataResult,0x0a,64);
		memset(dataResult+64,0xa0,64);
		dataStorage.Put(dataResult,128);
	
		DBG("ProbeSize = %d",dataStorage.Probe());
	
		memset(dataResult,0x01,256);
		dataStorage.Put(dataResult,256);
		
		DBG("ProbeSize = %d",dataStorage.Probe());
	
		memset(dataResult,0x02,512);
		dataStorage.Put(dataResult,512);
		
		DBG("ProbeSize = %d",dataStorage.Probe());
	
		memset(dataResult,0x01,1024);
		int gl = dataStorage.Get(dataResult,1024);
		DBG_MEMDUMP("SD DATA small:", dataResult, gl);
		DBG("ProbeSize = %d",dataStorage.Probe());
		gl = dataStorage.Get(dataResult,1024);
		DBG_MEMDUMP("SD DATA small:", dataResult, gl);
		DBG("ProbeSize = %d",dataStorage.Probe());
		gl = dataStorage.Get(dataResult,1024);
		DBG_MEMDUMP("SD DATA small:", dataResult, gl);
		DBG("ProbeSize = %d",dataStorage.Probe());
	}
	
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

#elif defined APP_CANV10
	DBG("APP_CANV10");
	
	GPSSensor *gps = new GPSSensor(p13,p14,4,250);
	IMUSensor *imu = new IMUSensor(p28,p27);
	
	gps->Start();
	imu->Start();
	gps->Run();
	imu->Run();
	
	while(1){
		Thread::wait(1000);	
	}
	
	gps->Stop();
	imu->Stop();

#elif defined APP_CANONAIR
	DBG("CAN APP %d",sizeof(int));
	
	newCalcPending = false;
	printThread = true;
	// Set a wdt with 60s timer
	MyWatchdog wdt(60);

	PyrnUSBModem modem;
	
	// Create sd calc access;
	//calcStorage = new sdStorage(p5,p6,p7,p8,0,100);
	calcStorage = new ramStorage(512);

	int dataSz = 0;
	int err = calcStorage->Read((char*)&dataSz,0,sizeof(int));
	if(err!=-1) {
		DBG("CHECK IF CALCULATOR STORED %d",dataSz);
		if((dataSz>0) and (dataSz<2048)) {
			err = calcStorage->Read(dataResult,1,dataSz);
			if(err!=-1) {
				DBG_MEMDUMP("CHECK",dataResult,dataSz);
			} else {
				DBG("ERROR Reading calcData in sector 1");
			}
		} else {
			DBG("ERROR Wrong calcData Size %d",dataSz);
		}
	} else {
		DBG("ERROR Reading calcSz in sector 0");
	}
	
	int connected = modem.connect("a2bouygtel.com","","");
	//int connected = 0;
	if(connected != 0) {
		DBG("Could not connect modem.. Now reset system ");
		NVIC_SystemReset();
	} else {
			
		wdt.Feed();
		
		// NTPClient
		NTPClient *ntp = new NTPClient();
		ntp->setTime("0.europe.pool.ntp.org");
		delete ntp;
		
		#define COM_HANDLER_THREAD_STACK_SIZE   3*1024
		unsigned char *sp = (unsigned char*) memAlloc.malloc(COM_HANDLER_THREAD_STACK_SIZE);


		// 3G (configure the system to return ASAP for the first connection)
		ComHandler comm(this,"AAAAAAAAAAAAAAA",ComHandler::TT_HALF,sp,COM_HANDLER_THREAD_STACK_SIZE);

		// CAN 
		CANInterface canItf;
		c = &canItf;
		//CANSniffer canSnif(&canItf);

		// The static list of sensors (GPS/IMU/...)
		IMUSensor *imu = new IMUSensor(p28,p27,250);
		DBG("ADD Sensor IMU");
		staticSensors.AddSensor(imu);
		GPSSensor *gps = new GPSSensor(p13,p14,4,1000);
		DBG("ADD Sensor GPS");
		staticSensors.AddSensor(gps);
		
		/*
		int dataSz = 0;
		calcStorage->Read((char*)&dataSz,0,sizeof(int));
		DBG("CHECK IF CALCULATOR STORED %d",dataSz);
		if((dataSz>0) and (dataSz<2048)) {
			calcStorage->Read(dataResult,1,dataSz);
			DBG_MEMDUMP("APPLY",dataResult,dataSz);
			//NewDynSensors(dataResult,dataSz);
		}
		*/
		
		// Request the first calculators
#ifdef CAN_SIMULATOR
		// Diags
		CANDiagCalculatorHeader SimuHdr;
		SimuHdr.hdrVer = 1;
		SimuHdr.diagCode = 0x6A;
		SimuHdr.speed = 500;
		SimuHdr.addrSrc = 0x652;
		SimuHdr.addrDst = 0x752;
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
		gps->Start();
		comm.Start();
		//DiagSensor6A->Start();
		canItf.Start();


		imu->Run();
		gps->Run();
		comm.Run();
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
						DBG_MEMDUMP("data",dataResult,len);
						if(!comm.AddResults(st,dataResult,len)){
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
							DBG_MEMDUMP("data",dataResult,len);
							if(!comm.AddResults(st,dataResult,len)){
								ERR("Result could not be writen");
							}
							
						}
					}
				}
				dynSensorAccess.unlock();
			}
			
			if(newCalcPending) {
				int dataSz = 0;
				DBG("APPLY SENSORS HERE");
				newCalcPending = false;
				calcStorage->Read((char*)&dataSz,0,sizeof(int));
				calcStorage->Read(dataResult,1,dataSz);
				DBG_MEMDUMP("APPLY",dataResult,dataSz);
				if(StopDynSensors()) {
					NewDynSensors(dataResult,dataSz);
				}
			}
			
		    Thread::wait(1000);
		}

#ifdef CAN_SIMULATOR
		Sim6A->Stop();
#endif

		imu->Stop();
		gps->Stop();
		//DiagSensor6A.Stop();   
		canItf.Stop();
	}
#endif

#ifdef BLINKER
	br.Stop();
	bg.Stop();
#endif

}

bool MainClass::StopDynSensors() {
	osStatus s = dynSensorAccess.lock(4000);
	if(s == osOK){
		DBG("Stop Dynamic Sensors");
		int i = 0;
		while(1) {
			CANDiagSensor6A *s = (CANDiagSensor6A*) dynamicSensors.PopLastSensor();
			if(s != NULL) {
				DBG("%s%d Stop+WaitEnd",s->GetSensorName(),i);
				s->Stop();
				DBG("================================");
				Thread::wait(3000);
				DBG("================================");
				DBG("%s%d Got Stopped",s->GetSensorName(),i);
				s->WaitEnd();
				DBG("%s%d Ended",s->GetSensorName(),i);
				i++;
			} else {
				DBG("Dynamic list is empty");
				break;
			}
			CANDiagCalculator *calc = s->GetDiagCalculator();
			CANCommunicator6A *com = s->GetCommunicator();
			DBG("Release Sensor");
			delete(s);
			DBG("Release ComHandler");
			delete(com);
			DBG("Release DiagCalculator");
			delete(calc);  
			memAlloc.PrintResume(false);
		}
		
		dynSensorAccess.unlock();
		return true;
	} else if(s == osEventTimeout) {
		DBG("Could not obtain lock to change the ongoing Diagnoses.. data will be dropped");
		return false;
	}
}

void MainClass::NewDynSensors(char *data, int dataSz) {
	osStatus s = dynSensorAccess.lock(4000);
	if(s == osOK){
		DBG("NewDynSensors - Create New Dynamic Sensors");
		uint8_t nCalc = 0;
		int offset = 0;
		char *pData = data;
		int inc = 0;
		CANDiagCalculator *DiagCalc = new CANDiagCalculator();
		while(1) {
			inc = DiagCalc->SetData((const char*)pData+offset);
			DBG_MEMDUMP("NewDynSensors - CALC DATA", DiagCalc->GetDataPointer(), sizeof(CANDiagCalculatorHeader));
			if(inc) {
				DBG("NewDynSensors - Incrementing pointer offset to %d",inc);
				offset += inc;
			} else {
				DBG("NewDynSensors - Calculator data are corrupted ... free memory and abort");
				delete(DiagCalc);
				break;
			}
			if(DiagCalc->Ready()) {
				uint8_t dcode = DiagCalc->GetDiagCode();
				if(dcode == 0x6A) {
					uint8_t ch = DiagCalc->GetPinH();
					uint8_t cl = DiagCalc->GetPinL();
					int bus = c->BusFromPins(ch,cl);
					DBG("NewDynSensors - Calculator is ready [%d/%d-%d]",ch,cl,bus);
					if(bus) {
							CANCommunicator6A *Diag6A = new CANCommunicator6A(c,bus,DiagCalc);
							CANDiagSensor6A *DiagSensor6A = new CANDiagSensor6A(DiagCalc,Diag6A);
							DiagSensor6A->Start();
							DiagSensor6A->Run();
							DBG("NewDynSensors - Adding Sensor %p to dynamic list",DiagSensor6A);
							dynamicSensors.AddSensor(DiagSensor6A);
					} else {
						DBG("NewDynSensors - Wrong BUS number (%d-%d-%d)... free memory and abort",ch,cl,bus);
						delete(DiagCalc);
						break;
					}
				} else {
					DBG("NewDynSensors - Unknown Calculator DiagCode(0x%02X)",dcode);
					delete(DiagCalc);
				}
			} else {
				DBG("NewDynSensors - Calculator is not ready, something got wrong  ... free memory and abort");
				delete(DiagCalc);
				break;
			}
			// Check for ending
			if(offset>=dataSz) {
				DBG("NewDynSensors - Successfully instanciate %d sensors",nCalc+1);
				break;
			} else {
				nCalc++;
			}
		}
	} else if(s == osEventTimeout) {
		DBG("NewDynSensors - Could not obtain lock to change the ongoing Diagnoses.. data will be dropped");
	} else {
		DBG("NewDynSensors - WHAT %d",s);
	}
}

void MainClass::event(int ID, void *data) {
	//DBG("event received");
	int dataSz = ID;
	char *pData = (char*)data;
	int err = -1;
	if(dataSz != 0) {
		//DBG_MEMDUMP("RX DATA", pData, dataSz);
		DBG("Store new calculator Size(%d)",dataSz);
		err = calcStorage->Write((char*)&dataSz,0,sizeof(int));
		if(err == -1) {
			DBG("Error While Writing calcSz in sector 0 ... now abort");
			return;
		}	
		err = calcStorage->Write(pData,1,dataSz);
		if(err == -1) {
			DBG("Error While Writing calcData in sector 1 ... now abort");
			return;
		}	
		newCalcPending = true;
	}
}

int main(void) {
	set_time(0);
    debug_init();
    debug_set_newline("\r\n");
    debug_set_speed(115200);
    
    printThread = true;
    
    // Main Function
    MainClass m(2,5);
    m.run();
    
    // Dead end that should never come
    while(1) Thread::wait(1000);
}

