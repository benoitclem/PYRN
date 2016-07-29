
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

#include "LSM303DLH.h"
#include "L3GD20H.h"
//#include "IMUSensor.h"
#include "GPSSensorNMEA.h"
#include "GPSSensorUBX.h"

#include "CANInterface.h"
#include "CANSniffer.h"
#include "CANCommunicator6A.h"
#include "CANDiagSensor.h"
#include "CANCorrelator.h"
#include "CANVariationDetector.h"

#include "CANRecorder.h"
#include "CANRecorderCalculator.h"

#include "PyrnUSBModem.h"
#include "uBloxUSBModem.h"
#include "HTTPClient.h"
#include "TCPSocketConnection.h"

#include "NTPClient.h"
#include "ComHandler.h"
#include "tcpDataClient.h"
#include "udpDataClient.h"

#include "storageBase.h"

#include "Configs.h"

#include "sd.h"

#define BLINKER
//#define THREAD_MONITOR
//#define APP_TEST
//#define APP_GPS_UBX_TEST
//#define APP_CANV10
//#define CAN_SIMULATOR
//#define APP_SHARKAN
//#define APP_STORAGE
//#define APP_SDSPI
//#define APP_CAN_FILTER_TEST_TX
//#define APP_CAN_FILTER_TEST_RX
//#define APP_CORRELATOR
//#define APP_CANONAIR
//#define APP_CANPLAYER
//#define APP_GPS
//#define APP_CAN_VARIATION
//#define APP_CANONAIR_V2 // This is the sniffing app (CAN Recorder)
#define APP_CANONAIR_V3G
bool printThread;

bool newCalcPending;
char dataResult[1024]  __attribute((section("AHBSRAM1")));
extern MyMemoryAllocator memAlloc;

storage *calcStorage;

ComHandler *com;

CANInterface *c;

//Serial pc(USBTX, USBRX); // tx, rx

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
#elif defined APP_GPS_UBX_TEST
	//DBG("TEST GPS UBX APP");
	GPSSensorNMEA *gps = new GPSSensorNMEA(p13,p14,900);
	//Serial gps0(p13, p14);  // tx, rx
	//gps0.printf("$PUBX,41,1,0007,0001,57600,0*29\r\n");
	//GPSSensorUBX *gps = new GPSSensorUBX(p13,p14,4,1000);
	//	gps->DoSerialAGPS();
	Thread::wait(1000);
	gps->Start();
	gps->Run();
	//DBG("caca");
	while(1) {
		Thread::wait(1000);
	}
	gps->Stop();
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

	CANInterface canItf(500000,3,8,500000,6,14);
	c = &canItf;
	//int32_t calcIds[2] = {0x728,0x720}; 	// Fiesta instrument IPC
	//int32_t calcIds[2] = {0x7e8,0x7e0};		// Fiesta essence 
	//int32_t calcIds[2] = {0x743,0x763};		// megane diesel 
	//int32_t calcIds[2] = {0x7e8,0x7e0};		// megane instruments

	//CANSniffer canSnif(&canItf,calcIds,2);
	CANSniffer canSnif(&canItf);

	canItf.Start();
	canItf.Run();

	
	while(1){
		Thread::wait(1000);
	}

	/*
	char dataOne[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	char dataTwo[5] = {0x41,0x42,0x43,0x44,0x45};
	while(true) {
		Thread::wait(10);
		dataOne[0] -= 3;
		dataOne[2] += 2;
		dataOne[3] += 1;
		dataTwo[4] -= 1; 
		canItf.Send(2,0X456,dataOne,8);
		Thread::wait(10);
		canItf.Send(2,0X476,dataTwo,5);
	}
	*/

	/*
	char dataOne[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	while(1){
		canItf.Send(2,0X456,dataOne,8);
		Thread::wait(1000);	
	}
	*/
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

#elif defined APP_CORRELATOR

	DBG("CORRELATOR APP");
	
	//printThread = true;

	CANInterface canItf;
	c = &canItf;
	//c->SetFilter(1,0x456);
	c->SetFilter(1,0x476);
	//CANSniffer canSnif(&canItf);
	CANCorrelator canBusCorrOne(&canItf,1);
	//CANCorrelator canBusCorrOne(&canItf,2);

	canItf.Start();
	canItf.Run();

	char dataOne[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	char dataTwo[5] = {0x41,0x42,0x43,0x44,0x45};
	while(true) {
		Thread::wait(1000);
		dataOne[0] -= 3;
		dataOne[2] += 2;
		dataOne[3] += 1;
		dataTwo[4] -= 1; 
		canItf.Send(2,0X456,dataOne,8);
		Thread::wait(1000);
		canItf.Send(2,0X476,dataTwo,5);
		canBusCorrOne.PrintVariables();
	}

	canItf.Stop();

#elif defined APP_CAN_FILTER_TEST_TX

	DBG("CAN FILTER TEST APP TX");
	
	//printThread = true;

	CANInterface canItf;
	c = &canItf;

	canItf.Start();
	canItf.Run();

	char dataOne[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	char dataTwo[5] = {0x41,0x42,0x43,0x44,0x45};
	char dataThree[7] = {0x51,0x52,0x53,0x54,0x55,0x56,0x57};
	char dataFour[6] = {0x61,0x62,0x63,0x64,0x65,0x66};
	char dataFive[3] = {0x21,0x22,0x23};
	char dataSix[2] = {0x11,0x12};
	while(true) {
		Thread::wait(100);
		canItf.Send(2,0x456,dataOne,8);
		Thread::wait(40);
		canItf.Send(2,0x476,dataTwo,5);
		Thread::wait(20);
		canItf.Send(2,0x534,dataThree,7);
		canItf.Send(2,0x627,dataFour,6);
		Thread::wait(60);
		canItf.Send(2,0x209,dataFive,3);
		canItf.Send(2,0x2A2,dataSix,2);
		DBG("RUNNING");
	}

	canItf.Stop();


#elif defined APP_CAN_FILTER_TEST_RX

	DBG("CAN FILTER TEST APP RX");
	
	//printThread = true;

	CANInterface canItf;
	c = &canItf;
	//c->SetFilter(1,0x456);
	//c->SetFilter(2,0x476);
	//c->SetFilter(2,0x627);
	
	//c->SetFilter(2,0x209);

	c->SetFilter(2,0x222);
	c->SetFilter(2,0x2A2);
	c->SetFilter(2,0x456);
	c->SetFilter(2,0x627);
	c->SetFilter(2,0x534);

	
	
	//
	CANSniffer canSnif(&canItf);

	canItf.Start();
	canItf.Run();

	while(true) {
		Thread::wait(1000);
		DBG("RUNNING");
	}

	canItf.Stop();

#elif defined APP_CANPLAYER
	DBG("CAN APP PLAYER");
	CANInterface canItf;
	c = &canItf;

	canItf.Start();
	canItf.Run();

	char dataOne[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	char dataTwo[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	while(true) {
		DBG("Loop");
		Thread::wait(80);
		dataOne[0] -= 3;
		dataOne[2] += 2;
		dataOne[3] += 1;
		dataTwo[0] -= 4;
		dataTwo[1] += 9;
		dataTwo[2] += 9;
		dataTwo[3] += 6;

		canItf.Send(2,0x208,dataOne,8);
		Thread::wait(20);
		canItf.Send(2,0x612,dataTwo,8);
	}
#elif defined APP_CAN_VARIATION

	DBG("VARIATION APP");
	
	//printThread = true;

	CANInterface canItf;
	c = &canItf;
	//c->SetFilter(1,0x456);
	//c->SetFilter(1,0x476);
	//CANSniffer canSnif(&canItf);
	CANVariationDetector variator(&canItf,1);
	//CANCorrelator canBusCorrOne(&canItf,2);

	canItf.Start();
	canItf.Run();

	char dataOne[8] = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
	char dataTwo[5] = {0x41,0x42,0x43,0x44,0x45};
	while(true) {
		Thread::wait(1000);
		dataOne[0] -= 3;
		dataOne[2] += 2;
		dataOne[3] += 1;
		dataTwo[4] -= 1; 
		canItf.Send(2,0X456,dataOne,8);
		Thread::wait(1000);
		canItf.Send(2,0X476,dataTwo,5);
	}

	canItf.Stop();
#elif defined APP_CANONAIR_V2
	DBG("CAN APP V2");

	// The canonair V2 is basically a sniffer. 
	// Server can specify which ID is listened
	// Also he can tell which bytes in frame to 
	// save

	MyWatchdog wdt(60);
	PyrnUSBModem modem;

	int connected = modem.connect("a2bouygtel.com","","");
	//int connected = 0;
	if(connected != 0) {
		DBG("Could not connect modem.. Now reset system ");
		NVIC_SystemReset();
	} else {
		wdt.Feed();
		CANInterface canItf;
		DBG("Connected");
		// 3G (configure the system to return ASAP for the first connection)
		// unsigned char *sp = (unsigned char*) memAlloc.malloc(COM_HANDLER_THREAD_STACK_SIZE);
		//tcpDataClient comm(sp,COM_HANDLER_THREAD_STACK_SIZE);
		udpDataClient comm;

		/*
		typedef struct _CANRecorderCalculatorHeader {
			uint8_t		hdrType;
			uint32_t	idCalc;
			uint16_t 	speed;
			uint32_t	rxAddr;
			uint8_t		pinh;
			uint8_t		pinl;
			uint8_t		nChunks;
			char 		data[];
		} __attribute__((packed)) CANRecorderCalculatorHeader;
		*/

		LSM303DLH  Accelerometer(p28,p27);
		GPSSensorNMEA *gps = new GPSSensorNMEA(p13,p14,1000);
		gps->Start();
		gps->Run();

		#define RECDTASZ 720

		char recordData[RECDTASZ];

		CANRecorderCalculator *recCalcs[32];
		CANRecorder *recs[32];
		uint8_t nMaxRecs = 32;
		uint8_t currRec = 0;


		// Step here to check in which calc setup we are
		char *config = recordData;
		uint32_t index = 0;
		int l = 0;

		/*
		int l = comm.GetTestIDs(config,RECDTASZ);
		DBG_MEMDUMP("TestIds",config,l);

		index = *((int*)config);

		DBG("TestIds %d",index);		

		CANSniffer *canSniff = new CANSniffer(&canItf);
		
		canItf.Start();
		canItf.Run();

		// Wait for an id (in index to reuse some variables) to change
		if(canSniff->wait(index,2000)){
			DBG("Got answer, this is configuration 0");	
			index = 0;
		} else {
			DBG("No answer, this is configuration 1");	
			index = 1;
		}

		canItf.Stop();

		delete(canSniff);

		*/

		DBG("Index config value is %d",index);
		l = comm.GetConfig(index,config,RECDTASZ);
		DBG_MEMDUMP("Config",config,l);

		/*
		0x10006cfc: 03 08 06 00 00 02 02 02 03 01 bb 05 00 00 01 04 ................
		0x10006d0c: 02 2e 01 00 00 03 00 01 03 01 04 01             ............  
		*/

		uint8_t nCalcs = *((uint8_t*)(config));
		//DBG("NCalcs = %d",nCalcs);
		int offset = 1;
		//char calcData[128];
		#define CALC_DATA_SZ 128
		char *calcData = (char*) memAlloc.malloc(sizeof(char)*CALC_DATA_SZ);
		CANRecorderCalculatorHeader *pCalc = (CANRecorderCalculatorHeader *) calcData;
		while(offset<l) {
			if(currRec<nMaxRecs){
				memset(calcData,0,CALC_DATA_SZ);
				pCalc->hdrType = 2;
				pCalc->idCalc = 0x0;
				pCalc->speed = 500;
				pCalc->rxAddr = *((uint32_t*)(config+offset));
				offset += 4;
				pCalc->pinh = 6;
				pCalc->pinl = 14;
				pCalc->nChunks = *((uint8_t*)(config+offset));
				offset += 1;
				DBG("rxAddr %08x, nChunks %d",pCalc->rxAddr,pCalc->nChunks);
				for(int c = 0; c <pCalc->nChunks; c++) {
					pCalc->data[c*2] = *((uint8_t*)(config+offset));
					pCalc->data[(c*2)+1] = *((uint8_t*)(config+offset+1));
					offset += 2;
				}

				CANRecorderCalculator *calc = new CANRecorderCalculator(calcData);
				if(calc->Ready()) {
					recCalcs[currRec] = calc;
					recs[currRec] = new CANRecorder(&canItf,recCalcs[currRec]);
					currRec++;
				} else {
					delete(calc);
				}
			} else {
				DBG("Reached max recorders");
				break;
			}
		}
		memAlloc.free(calcData);

		DBG("Got %d calculators allocated",currRec);
		
		PrintActiveThreads();
		for(int p = 0; p<nCalcs; p++){
			DBG("PROUT %d %d",recs[p]->getNPoints(),recs[p]->getNPointsMax());
		} 

		//CANSniffer canSnif(&canItf);
		
		canItf.Start();
		//comm.Start();
		
		canItf.Run();
		//comm.Run();

		uint16_t pktCount = 0;

		PrintActiveThreads();
		for(int p = 0; p<nCalcs; p++){
			DBG("PROUT %d %d",recs[p]->getNPoints(),recs[p]->getNPointsMax());
		} 

		while(1) {
			// Buffer receiving the captured data
			char buff[64];
			// Final Buffer received the recorded data
			memset(recordData,0,RECDTASZ);

			//DBG("MainThread Loop");
			wdt.Feed();
			Thread::wait(200);
			uint16_t offset = 0;
			uint16_t len;

			// Tell this is a frame 
			*((uint8_t*)(recordData+offset)) = 0x02;
			offset += 1;

			// Packet counter
			*((uint16_t*)(recordData+offset)) = pktCount;
			pktCount += 1;
			offset += 2;

			// Do the capture
			for(uint8_t i = 0; i<currRec; i++) {
				DBG("capture data %d %p",i,recs[i]);
				len = 64;
				recs[i]->Capture(buff,&len);
				if(len!=0) {
					DBG_MEMDUMP("capturedData",buff,len);
					*((uint16_t*)(recordData+offset)) = len;
					offset += 2;
					memcpy(recordData+offset,buff,len);
					offset += len;
				} else {
					WARN("Recorder still learning dt or no impact");
				}
			}

			// IMU SAMPLING
			float lax,lay,laz,lmx,lmy,lmz;              // Locals
    		Accelerometer.readAcc(&lax,&lay,&laz);
    		Accelerometer.readMag(&lmx,&lmy,&lmz);
    		//DBG("%3.4f %3.4f %3.4f %3.4f %3.4f %3.4f",lax,lay,laz,lmx,lmy,lmz);
    		// Build a packet with this data

    		// Network Frame Structure is id(4) | dt (2) | n (2) | nChunk(1) | entries (n*2) | data (n)
    		typedef struct _imuFrame{
    			uint16_t sz; 	// 2
    			uint32_t id; 	// 4
    			uint16_t dt;	// 2
    			uint16_t n; 	// 2
    			uint8_t nChunk; // 1
    			uint8_t startax;	// 1
    			uint8_t lenax;    // 1
    			uint8_t startay;	// 1
    			uint8_t lenay;    // 1
    			uint8_t startaz;	// 1
    			uint8_t lenaz;    // 1
    			uint8_t startmx;	// 1
    			uint8_t lenmx;    // 1
    			uint8_t startmy;	// 1
    			uint8_t lenmy;    // 1
    			uint8_t startmz;	// 1
    			uint8_t lenmz;    // 1
    			float ax;	// 4
    			float ay;	// 4
    			float az;	// 4
    			float mx;	// 4
    			float my;	// 4
    			float mz;	// 4
    		} __attribute__((packed)) imuFrame;

    		imuFrame *f = (imuFrame*) (recordData+offset);
    		f->sz = sizeof(imuFrame)-2;
    		f->id = 0xA5A5A5A5;
    		f->dt = 0;
    		f->n = 24;
    		f->nChunk = 6;
			f->startax = 0;	// 1
			f->lenax = 4;    // 1
			f->startay = 1;	// 1
			f->lenay = 4;    // 1
			f->startaz = 2;	// 1
			f->lenaz = 4;   // 1
			f->startmx = 3;	// 1
			f->lenmx = 4;    // 1
			f->startmy = 4;	// 1
			f->lenmy = 4;    // 1
			f->startmz = 5;	// 1
			f->lenmz = 4;    // 1
    		f->ax = lax;
    		f->ay = lay;
    		f->az = laz;
    		f->mx = lmx;
    		f->my = lmy;
    		f->mz = lmz;

    		DBG_MEMDUMP("imuDataFrame",recordData+offset,sizeof(imuFrame));

    		offset+= sizeof(imuFrame);

    		// GPS
    		GPSSensorNMEA::gpsImpact impact;
    		bool r = gps->GetImpact(&impact);
    		if(r){
    			//DBG_MEMDUMP("gpsImpact",(char*)&impact,sizeof(GPSSensor::gpsImpact));

    			/*
    			uint32_t date;
		        uint32_t time;
		        int32_t lon;
		        int32_t lat;
		        int32_t alt;
		        uint16_t hdop;
		        */

	    		typedef struct _gpsFrame{
	    			uint16_t sz; 	// 2
	    			uint32_t id; 	// 4
	    			uint16_t dt;	// 2
	    			uint16_t n; 	// 2
	    			uint8_t nChunk; // 1

	    			uint8_t startdate;	// 1
	    			uint8_t lendate;    // 1

	    			uint8_t starttime;	// 1
	    			uint8_t lentime;    // 1

	    			uint8_t startlon;	// 1
	    			uint8_t lenlon;    // 1

	    			uint8_t startlat;	// 1
	    			uint8_t lenlat;    // 1

	    			uint8_t startalt;	// 1
	    			uint8_t lenalt;    // 1

	    			uint8_t starthdop;	// 1
	    			uint8_t lenhdop;    // 1
	    			GPSSensorNMEA::gpsImpact gpsi; // sizeof(GPSSensor::gpsImpact)
	    		} __attribute__((packed)) gpsFrame;

	    		gpsFrame *f = (gpsFrame*) (recordData+offset);
	    		f->sz = sizeof(gpsFrame) - 2;
	    		f->id = 0x5A5A5A5A;
	    		f->dt = 0;
	    		f->n = sizeof(GPSSensorNMEA::gpsImpact);
	    		f->nChunk = 6;
    			f->startdate = 0;	// 1
    			f->lendate = 4;    // 1
    			f->starttime = 1;	// 1
    			f->lentime = 4;    // 1
    			f->startlon = 2;	// 1
    			f->lenlon = 4;    // 1
    			f->startlat = 3;	// 1
    			f->lenlat = 4;    // 1
    			f->startalt = 4;	// 1
    			f->lenalt  = 4;    // 1
    			f->starthdop = 5;	// 1
    			f->lenhdop = 2;

	    		memcpy(&(f->gpsi),&impact,sizeof(GPSSensorNMEA::gpsImpact));

	    		offset+= sizeof(gpsFrame);

    		} else {
    			//DBG("Could not get gps impact");
    		}

    		// GPS Position

			DBG_MEMDUMP("ALL CAPTURE DATA", recordData, offset);

			comm.SendData(recordData,offset);
		}	
	}

#elif defined APP_CANONAIR_V3G

	Thread::wait(1000);
	DBG("APP 3G Test");

	I2C dev(p28,p27);
	INFO("I2C Init (400kHz)");
    dev.frequency(400000);

	LSM303DLH  Accelerometer(&dev);
	L3GD20H Gyroscope(&dev);

	while(1) {
		float lax,lay,laz,lmx,lmy,lmz,lgx,lgy,lgz;              // Locals
		Accelerometer.readAcc(&lax,&lay,&laz);
		Accelerometer.readMag(&lmx,&lmy,&lmz);
		Gyroscope.readGyr(&lgx,&lgy,&lgz);
		DBG("%3.4f %3.4f %3.4f %3.4f %3.4f %3.4f %3.4f %3.4f %3.4f",lax,lay,laz,lmx,lmy,lmz,lgx,lgy,lgz);
		Thread::wait(100);
	}


	uBloxUSBModem modem = uBloxUSBModem(p22,p21);
	if(modem.init()) {
		DBG("Got SARA U2 Modem");
		int connected = modem.connect("sl2sfr","","");
		if(connected != 0) {
			DBG("Nope, we could not connect");
		} else {
			char recordData[1024];
			DBG("Everything goes right");
			udpDataClient comm;
			uint32_t index = 0;
			int l = comm.GetConfig(index,recordData,1024);
			DBG_MEMDUMP("Config",recordData,l);

		}

	} else {
		DBG("Could not connect SARA U2 Modem");	
	}
	//

	//modem.PulseOnOff();
	Thread::wait(5000);

	GPSSensorNMEA *gps = new GPSSensorNMEA(p13,p14,1000);
	gps->Start();
	gps->Run();

	while(1)
		Thread::wait(100);

	gps->Stop();

#elif defined APP_GPS
	DBG("GPS APP");

	MyWatchdog wdt(60);

	wdt.Feed();
	DBG("Connected");

	GPSSensor *gps = new GPSSensor(p13,p14,4,500);
	gps->Start();
	gps->Run();

	while(1) {
		wdt.Feed();

		// GPS
		GPSSensor::gpsImpact impact;
		bool r = gps->GetImpact(&impact);
		if(r){
			DBG("IMPACT");
		} else {
			DBG("Could not get gps impact");
		}

		Thread::wait(100);
	}	

#elif defined APP_CANONAIR
	DBG("CAN APP");
	
	newCalcPending = false;
	printThread = true;
	// Set a wdt with 60s timer
	MyWatchdog wdt(60);

	/*
	GPSSensorNMEA *gps = new GPSSensorNMEA(p13,p14,4,1000);
	gps->Start();
	gps->Run();

	while(1) Thread::wait(1);
	*/

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

		long stackpointer asm ("sp");

		DBG("SP: %d",stackpointer);
		
		// NTPClient
		NTPClient *ntp = new NTPClient();
		ntp->setTime("0.europe.pool.ntp.org");
		delete ntp;
		
		// 3G (configure the system to return ASAP for the first connection)
		unsigned char *sp = (unsigned char*) memAlloc.malloc(COM_HANDLER_THREAD_STACK_SIZE);
		ComHandler comm(this,"AAAAAAAAAAAAAAA",ComHandler::TT_HALF,sp,COM_HANDLER_THREAD_STACK_SIZE);

		com = &comm;

		// CAN 
		CANInterface canItf;
		c = &canItf;
		//CANSniffer canSnif(&canItf);

		// The static list of sensors (GPS/IMU/...)
		IMUSensor *imu = new IMUSensor(p28,p27,250);
		DBG("ADD Sensor IMU");
		staticSensors.AddSensor(imu);
		GPSSensorNMEA *gps = new GPSSensorNMEA(p13,p14,4,1000);
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
			MySensorBase *sb = (MySensorBase*) dynamicSensors.PopLastSensor();
			if(s != NULL) {
				uint8_t type = sb->GetSensorType();
				if(type == SENSOR_TYPE_CAN_DIAG) {
					// This is the CAN Diag Stuffs
					// Need to stop sensors and wait end
					CANDiagSensorBase *s = (CANDiagSensorBase*) sb;
					DBG("%s%d Stop+WaitEnd",s->GetSensorName(),i);
					s->Stop();
					DBG("================================");
					Thread::wait(3000);
					DBG("================================");
					DBG("%s%d Got Stopped",s->GetSensorName(),i);
					s->WaitEnd();
					DBG("%s%d Ended",s->GetSensorName(),i);
					i++;
					// Need to release the n Calculators
					DBG("There is N DiagCalculator (%d)",s->GetNDiagCalculator());
					for(int i = 0; i<s->GetNDiagCalculator(); i++) {
						CANDiagCalculator *calc = s->GetDiagCalculator(i);
						DBG("|-> Release DiagCalculator (%d)",i);
						delete(calc);  
					}
					// Need to release the Sensor and communicator
					CANCommunicatorMaster *com = s->GetCommunicator();
					DBG("Release Sensor");
					delete(s);
					DBG("Release ComHandler");
					delete(com);
					memAlloc.PrintResume(false);
				} else if(type == SENSOR_TYPE_CAN_REC){
					CANRecorder *s = (CANRecorder*) sb;
					s->DeActivate();
					Thread::wait(100);
					CANRecorderCalculator *calc = s->GetCalculator();
					DBG("Release sensor");
					delete(s);
					DBG("Release calculator");
					delete(calc);
				}
			} else {
				DBG("Dynamic list is now empty");
				break;
			}
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
		while(1) {
			// In this part we can create DIAGNOSES SENSORS
			if(*((const char*)(pData+offset)) == CAN_DIAG_HDR_TYPE) {
				CANDiagCalculator *DiagCalc = new CANDiagCalculator();
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
					if((dcode == 0x6A)||(dcode == 0x68)) {
						uint8_t ch = DiagCalc->GetPinH();
						uint8_t cl = DiagCalc->GetPinL();
						int bus = c->BusFromPins(ch,cl);
						DBG("NewDynSensors - Calculator is ready [%d/%d-%d]",ch,cl,bus);
						if(bus) {
							DBG("NewDynSensors - Check the sensors");
							int nS = dynamicSensors.GetNSensors();
							int mS = dynamicSensors.GetMaxSensors();
							if(nS<mS){
								DBG("NewDynSensors - Can add a sensor");
								CANCommunicatorMaster *diagBase;
								CANDiagSensorBase *diagSensorBase;
								if(dcode == 0x6A){
									diagBase = new CANCommunicator6A(c,bus,DiagCalc);
									diagSensorBase = new CANDiagSensor6A(DiagCalc,(CANCommunicator6A*)diagBase);
								} else if(dcode == 0x68) {
									diagBase = new CANCommunicator68(c,bus,DiagCalc);
									diagSensorBase = new CANDiagSensor68(DiagCalc,(CANCommunicator68*)diagBase);
								}
								diagSensorBase->Start();
								diagSensorBase->Run();
								DBG("NewDynSensors - Adding Sensor %p to dynamic list",diagSensorBase);
								dynamicSensors.AddSensor(diagSensorBase);
							} else {
								bool done = false;
								DBG("NewDynSensors - Try to add a diag calc to a existing sensor");
								for(int i = 0; i < mS; i++) {
									CANDiagSensorBase *s = (CANDiagSensorBase*) dynamicSensors.GetSensor(i);
									if(s != NULL){
										int n = s->GetNDiagCalculator();
										if(n<2) {
											DBG("NewDynSensors - Adding a diag calc to a existing sensor (%d)(%d)",i,n);
											s->AddCalculator(DiagCalc,n);
											done = true;
											break;
										}
									}
								}
								if(!done) {
									DBG("NewDynSensors - Could not add the new sensor");
									delete(DiagCalc);
								}
							}
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
			// In this part we create the RECORDER STUFFS
			} else if(*((const char*)(pData+offset)) == CAN_RECORDER_HDR_TYPE) {
				CANRecorderCalculator *RecordCalc = new CANRecorderCalculator();
				inc = RecordCalc->SetData((const char*)pData+offset);
				DBG_MEMDUMP("NewDynSensors - CALC RECORD DATA", RecordCalc->GetDataPointer(), sizeof(CANDiagCalculatorHeader));
				if(inc) {
					DBG("NewDynSensors - Incrementing pointer offset to %d",inc);
					offset += inc;
					CANRecorder *RecordSensor = new CANRecorder(c,RecordCalc);
					dynamicSensors.AddSensor(RecordSensor);
				} else {
					DBG("NewDynSensors - Calculator data are corrupted ... free memory and abort");
					delete(RecordCalc);
					break;
				}
				
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
	int dataSz = ID-40;
	char *pData = (char*)data;
	int err = -1;
	com->setIdConfig(pData);
	pData += 40;
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
    //debug_set_speed(9600);
    debug_set_speed(230400);
    //debug_set_speed(460800);
    //debug_set_speed(921600);
    
    printThread = true;
    
    // Main Function
    MainClass m(2,5);
    m.run();
    
    // Dead end that should never come
    while(1) Thread::wait(1000);
}


