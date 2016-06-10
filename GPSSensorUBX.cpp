
#include "GPSSensorUBX.h"

#define __DEBUG__ GPS_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "GPSSensorUBX.cpp"
#endif

#include "MyDebug.h"

#define swBytes(x) (((x&0xff00)>>8)|((x&0x00ff)<<8))

// MODSERIAL IRQ

void ubxRxFullCallback(MODSERIAL_IRQ_INFO *q){
    DBG("OVF");
    MODSERIAL *serial = q->serial;
    serial->rxBufferFlush();
}

void ubxRxCallback(MODSERIAL_IRQ_INFO *q) {
    MODSERIAL *serial = q->serial;
    DBG("%c",serial->rxGetLastChar());
    /*if ( serial->rxGetLastChar() == '\n') {
        //nmeaGpsNewlineDetected = true;
    }*/
}

/* ## NMEA CheckSum
def cs(t):
	xor = 0
	for c in t:
		xor = xor ^ ord(c)
	return xor
*/

GPSSensorUBX::GPSSensorUBX(PinName tx, PinName rx, uint32_t idle, uint32_t stackSz, unsigned char* sp):
    GPSSensor(tx, rx, idle, stackSz, sp) {
    gps.baud(9600);
    gps.rxBufferFlush();
    gps.txBufferFlush();
    WARN("CUT NMEA Frames");
    gps.printf("$PUBX,41,1,7,1,9600,0*12\r\n");
    //gps.baud(57600);
    Thread::wait(500);
    gps.rxBufferFlush();
    gps.txBufferFlush();
    WARN("Setup Callbacks");
    gps.attach(&ubxRxCallback, MODSERIAL::RxIrq);
    gps.attach(&ubxRxFullCallback, MODSERIAL::RxOvIrq);
    WARN("Finish Init");
}

GPSSensorUBX::~GPSSensorUBX(){
	
}

void GPSSensorUBX::ComputeCheckSum(unsigned char* buffer, int size, unsigned char* checkSumA, unsigned char* checkSumB) {
    *checkSumA = 0;
    *checkSumB = 0;
    for(int i = 0; i < size; i++) {
        *checkSumA += buffer[i];
        *checkSumB += *checkSumA;
    }
}

void GPSSensorUBX::AppendCheckSum(UBXMsg *msg) {
	unsigned char chA;
    unsigned char chB;
    unsigned char *buff = (unsigned char*)(&(msg->hdr));
    int len = (msg->hdr.length)+4;
    //DBG_MEMDUMP("Before CS",(char*)msg,len+2);
    ComputeCheckSum(buff,len,&chA,&chB);
    buff[len] = chA;
    buff[len+1] = chB;
    //DBG_MEMDUMP("AFTER CS",(char*)msg,len+4);
}

void GPSSensorUBX::Sample(void) {
	uint32_t hAcc,vAcc;
	//GetTime(&impact.date,&impact.time);
	GetPosition(&impact.lat,&impact.lon,&impact.alt,&hAcc,&vAcc);
}

void GPSSensorUBX::SendFrame(char *f, int size) {
	for(int i = 0; i < size; i++){
		//DBG("Tx");
		gps.putc(f[i]);
	}
}

int GPSSensorUBX::ReceiveFrame(uint32_t timeOutUs){
	bool reading = false;
	int i = 0;
	int j = 0;
	int jMax = 10;  
	uint32_t deadLine = us_ticker_read() + timeOutUs;
	while(1) {
		//DBG("PIPI");
		if(gps.readable()) {
			j = 0;
			//DBG("Rx");
			recvBuff[i++] = gps.getc();
			reading = true;
		} else {
			// DBG("CACA");
			// Was reading and become unreadable
			if(reading) {
				if(j<jMax) {
					j++;
					Thread::wait(4);
				} else {
					DBG_MEMDUMP("Received", (const char*) recvBuff, i);
					return i;
				}
			} else {
				if(us_ticker_read()>deadLine) {
					DBG("TimedOut");
					return 0;
				} else {
					Thread::wait(10);
					//DBG("ZIZI");
				}
			}
		}
	}
}

void GPSSensorUBX::FillHeader(UBXMsg *msg, UBXMessageClass msgClass, UBXMessageId msgId, int len) {
    msg->preamble = swBytes(UBX_PREAMBLE);
    msg->hdr.msgClass = msgClass;
    msg->hdr.msgId = msgId;
    msg->hdr.length = len;
}

void GPSSensorUBX::PollCmd(UBXMessageClass msgClass, UBXMessageId msgId) {
	UBXMsg *msg = (UBXMsg*)recvBuff;
	FillHeader(msg,msgClass,msgId,0);
    AppendCheckSum(msg);
    SendFrame((char*)msg,(msg->hdr.length)+UBX_OVERHEAD_SIZE);
}

void GPSSensorUBX::MON_VER(void) {
	DBG("TX checkVersion\n");
    PollCmd(UBXMsgClassMON, UBXMsgIdMON_VER);
}

void GPSSensorUBX::NAV_POSLLH(void) {
	//DBG("Tx NavPos\n");
	PollCmd(UBXMsgClassNAV, UBXMsgIdNAV_POSLLH);
}

void GPSSensorUBX::NAV_RATE(void) {
	DBG("TX check Nav Rate\n");
    PollCmd(UBXMsgClassCFG, UBXMsgIdCFG_RATE );
}

void GPSSensorUBX::DoSerialAGPS(void) {
	// Light 
	PwmOut led1(LED1);
	PwmOut led2(LED2);
	led1.write(1.0);
	led2.write(1.0);
	bool gotSomething = false;
	while(1) {
		char c;		
		if(debug_read(&c)) {
			DBG("got %c",c);
			gps.putc(c);
			gotSomething = true;
		} else if(gotSomething) {
			break;
		}
	}
	DBG("out");
	led1.write(0.0);
	led2.write(0.0);
}

void GPSSensorUBX::DoNetworkAGPS(char *servName) {
	// HTTPClient http;
	// Do it by chunk avoiding to alloc huge buffer into stack?
}

bool GPSSensorUBX::GetTime(uint32_t *date,uint32_t *time) {
	DBG("TX TIMEUTC");
	PollCmd(UBXMsgClassNAV, UBXMsgIdNAV_TIMEUTC);
	DBG("RX TIMEUTC");
	if(ReceiveFrame(10000)){
		if((recvBuff[0] == 0xb5) && (recvBuff[1] == 0x62)) {
			DBG("There is a Frame in buffer");
			if((recvBuff[2] == 0x01) && (recvBuff[3] == 0x21)) {
				DBG("OK This is a TIMEUTC Frame");
				UBXNAV_TIMEUTC *pTimeUTC= (UBXNAV_TIMEUTC*)(recvBuff+6);
				*date = (pTimeUTC->year>2000)?(pTimeUTC->year-2000):pTimeUTC->year;
				*date += (pTimeUTC->month * 100);
				*date += (pTimeUTC->day * 10000);
				*time = pTimeUTC->sec;
				*time *= pTimeUTC->min;
				*time *= pTimeUTC->hour;
				return true;
			}
		}
	}
	return false;
}

bool GPSSensorUBX::GetPosition(int32_t *lat, int32_t *lon, int32_t *alt, uint32_t *hAcc, uint32_t *vAcc) {
	//DBG("TX NAVPOS");
	PollCmd(UBXMsgClassNAV, UBXMsgIdNAV_POSLLH);
	//DBG("RX NAVPOS");
	if(ReceiveFrame(10000)){
		if((recvBuff[0] == 0xb5) && (recvBuff[1] == 0x62)) {
			//DBG("There is a Frame in buffer");
			if((recvBuff[2] == 0x01) && (recvBuff[3] == 0x02)) {
				DBG("OK this is a POSLLH frame");
				UBXNAV_POSLLH *pPosLlH = (UBXNAV_POSLLH*)(recvBuff+6);
				*lat = pPosLlH->lat;
				*lon = pPosLlH->lon;
				*alt = pPosLlH->height;
				*hAcc = pPosLlH->hAcc;
				*vAcc = pPosLlH->vAcc;
				DBG("%08x %08x %08x %08x %08x",*lat,*lon,*alt,*hAcc,*vAcc);
				DBG("%ld %ld %ld %ld %ld",*lat,*lon,*alt,*hAcc,*vAcc);
				if((*lat != 0) & (*lon != 0)) {
					DBG("VALID POS");
					fixed = true;
					return true;
				}
			}
		}
	}
	return false;
} 

bool GPSSensorUBX::GetDop(void) {
	return false;
}

/*
void GPSSensorUBX::ConfiguratePort(int bRate) {
    UBXMsg *msg = (UBXMsg*)recvBuff;
    FillHeader(msg,UBXMsgClassCFG,UBXMsgIdCFG_PRT,20);
    msg->payload.CFG_PRT.portID = 1;
    msg->payload.CFG_PRT.txReady.en = 0;   						// Disable txReady Pin
    msg->payload.CFG_PRT.mode.UART.charLen = UBXPRTMode8BitCharLen; 	// 8b
    msg->payload.CFG_PRT.mode.UART.parity = UBXPRTModeNoParity;  	// N(o)
    msg->payload.CFG_PRT.mode.UART.nStopBits = UBXPRTMode1StopBit; 	// 1Stop
    msg->payload.CFG_PRT.option.UARTbaudRate = bRate; 			// BaudRate
    msg->payload.CFG_PRT.inProtoMask = UBXPRTInProtoInUBX; 	// Only UBX
    msg->payload.CFG_PRT.outProtoMask = UBXPRTOutProtoOutUBX; 	// Only UBX
    AppendCheckSum(msg);
    SendFrame((char*)msg,(msg->hdr.length)+UBX_OVERHEAD_SIZE);
}*/