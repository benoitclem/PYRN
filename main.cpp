
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

#define BLINKER
//#define THREAD_MONITOR
//#define APP_TEST
//#define APP_SHARKAN
//#define CAN_SIMULATOR

char dataResult[1024] __attribute((section("AHBSRAM0")));

extern MyMemoryAllocator memAlloc;
CANInterface *c;

void threadMonitor(void const *args) {
    while (true) {
        PrintActiveThreads();
        Thread::wait(500);
    }
}

MainClass::MainClass(uint8_t nStaticts, uint8_t  nDynamics):
 staticSensors(nStaticts, true),
 dynamicSensors(nDynamics){

}

void MainClass::run(void) {

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
	
#elif defined APP_SHARKAN
	DBG("SHARKAN APP");

	CANInterface canItf;
	c = &canItf;
  CANSniffer canSnif(&canItf);

  canItf.Start();
  canItf.Run();

	while(1){
		Thread::wait(1000);	
	}

  canItf.Stop();

#else
	DBG("CAN APP");

	PyrnUSBModem modem;
	
	int connected = modem.connect("a2bouygtel.com","","");
	//int connected = 0;
	if(connected != 0) {
		DBG("Could not connect modem.. Now reset system ");
		NVIC_SystemReset();
	} else {
	
		// NTPClient
		NTPClient ntp;
		ntp.setTime("0.europe.pool.ntp.org");
	
		// Set a wdt with 60s timer
		MyWatchdog wdt(60);

		// 3G
		ComHandler comm(this,"AAAAAAAAAAAAAAA");
		//ComHandler comm(this,"012345678901234");

		// CAN 
		CANInterface canItf;
		c = &canItf;
		//CANSniffer canSnif(&canItf);

		// The static list of sensors (GPS/IMU/...)
		IMUSensor imu(p28,p27);
		staticSensors.AddSensor(&imu);
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
		/*
		char DiagCmd[38] = {CMD_ACK,		0,3,0x30,0x00,0x0A,
							CMD_KEEP_ALIVE,	0,2,0x01,0x3e,   
							CMD_NORM,		0,3,0x02,0x10,0xC0,
							CMD_NORM,		0,3,0x02,0x21,0x80,
							CMD_NORM,		0,3,0x02,0x21,0xC0,
							CMD_DIAG_START, 0,4,0x03,0x03,0x02,0x01,
							CMD_END,		0};
		DBG("SZ = %d",sizeof(DiagCmd));
		CANDiagCalculatorHeader DiagHdr;
		DiagHdr.hdrVer = 1;
		DiagHdr.diagCode = 0x6A;
		DiagHdr.speed = 500;
		DiagHdr.addrSrc = 0x74C;
		DiagHdr.addrDst = 0x64C;
		DiagHdr.pinh = 3;
		DiagHdr.pinl = 8;
		DiagHdr.cmdLen = sizeof(DiagCmd);
		char *DiagData = (char*) malloc (sizeof(CANDiagCalculatorHeader)+sizeof(DiagCmd));
		memcpy(DiagData,&DiagHdr,sizeof(CANDiagCalculatorHeader));
		memcpy(DiagData+sizeof(CANDiagCalculatorHeader),DiagCmd,sizeof(DiagCmd));
		
		int totalSz = sizeof(CANDiagCalculatorHeader)+ sizeof(DiagCmd);*/
		
		//this->event(totalSz,DiagData);
				
		// Simu Calculator
		CANDiagCalculator *simuCalc = new CANDiagCalculator((const char*)SimuData);
		//delete(simuCalc);
		//CANDiagCalculator simuCalc((const char*)SimuData);
		
		// Comm handlers
		uint8_t bus = canItf.BusFromPins(simuCalc->GetPinH(),simuCalc->GetPinL());
		CANCommunicatorSim6A *Sim6A = new CANCommunicatorSim6A(&canItf,bus,simuCalc);
		Sim6A->Start();
		//CANCommunicatorSim6A Sim6A(&canItf,2,simuCalc);
#endif
		/*
		// Diag
		CANDiagCalculator *DiagCalc = new CANDiagCalculator((const char*)DiagData);
		DBG("DiagCalc %p",DiagCalc);
		//delete(simuCalc);
		//CANDiagCalculator DiagCalc((const char*)DiagData);
	
		CANCommunicator6A *Diag6A = new CANCommunicator6A(&canItf,2,DiagCalc);
		DBG("Diag6A %p",Diag6A);
		//CANCommunicator6A Diag6A(&canItf,1,&);

		// Diagnoses
		CANDiagSensor6A *DiagSensor6A = new CANDiagSensor6A(DiagCalc,Diag6A);
		DBG("DiagSensor6A %p",DiagSensor6A);
		dynamicSensors.AddSensor(DiagSensor6A);
		
		*/

		// Threads
		imu.Start();
		//gps.Start();
		comm.Start();
		//DiagSensor6A->Start();
		canItf.Start();


		imu.Run();
		//gps.Run();
		comm.Run();
		//DiagSensor6A->Run();
		canItf.Run();
		
		Thread::wait(2000);

		// ecrire dans 5 et 6
		//char cbuff[8] = {'C','A','N','1','[',' ',' ',']'};
		//uint16_t loop = 0;
		uint16_t len = 0;
		
		/*while(1){
			wdt.Feed();
		
			DBG("MainThread");
		    // Fill up the cbuff
		    cbuff[5] = 0xff & (loop>>8);
		    cbuff[6] = 0xff & loop;
		    //PrintActiveThreads();
		    Thread::wait(1000);
		    time_t seconds = time(NULL);
			
		    //if((loop % 20) == 0) {
		    //	gps.Capture(dataResult,&len);
		    //	DBG("GPS[%04d] Got %03d chars",seconds,len);
			//	comm.AddResults(0xca,cbuff,8);
		    //}
		    //if((loop % 30) == 0) {
		    //	imu.Capture(dataResult,&len);
		    //	DBG("IMU[%04d] Got %03d chars",seconds,len);
		    //}
		    canItf.Send(1,0x666,cbuff,8);
			comm.AddResults(0xca,cbuff,8);
		    loop++;
		}*/
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
						comm.AddResults(st,dataResult,len);
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
							comm.AddResults(st,dataResult,len);
						}
					}
				}
				dynSensorAccess.unlock();
			}
		    // Fill up the cbuff
		    //cbuff[5] = 0xff & (loop>>8);
		    //cbuff[6] = 0xff & loop;
		    //PrintActiveThreads();
		    Thread::wait(1000);
		    //time_t seconds = time(NULL);
		    //canItf.Send(1,0x666,cbuff,8);
			//comm.AddResults(0xca,cbuff,8);
			//loop++;
			/*if(loop%15 == 0) {
				this->event(totalSz,DiagData);
				DBG("OOOOOOOOOOOOOUUUUUUUUUUUUUUUTTTTTTTTTTTTTTT");
			}*/
		}
		
		//Sim6A.Stop();

		//imu.Stop();
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
			memAlloc.free(s);
			DBG("Release ComHandler");
			memAlloc.free(com);
			DBG("Release DiagCalculator");
			memAlloc.free(calc);
			memAlloc.PrintResume();
		}
		DBG("Dynamic list is empty");
		uint8_t nCalc = 0;
		char *pData = (char*) data;
		int offset = 0;
		int inc = 0;
		int dataSz = ID;
		DBG_MEMDUMP("RX DATA", pData, dataSz);
		while(1) {
			CANDiagCalculator *DiagCalc = new CANDiagCalculator();
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
}

int main(void) {
	set_time(0);
    debug_init();
    debug_set_newline("\r\n");
    debug_set_speed(115200);
    
    // Main Function
    MainClass m(2,5);
    m.run();
    
    // Dead end
    while(1) Thread::wait(1000);
}

