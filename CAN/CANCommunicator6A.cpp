
#include "CANCommunicator6A.h"
#include "CANDiagCalculator.h"
#include "CANCommon.h"
#include "MyCallBackIds.h"
#include "MyOsHelpers.h"
#include "Configs.h"

#define __DEBUG__ CAN_COMM_DEBUG_LVL
#ifndef __MODULE__
#define __MODULE__ "CANCommunicator6A.cpp"
#endif
#include "MyDebug.h"


// The MASTER Stuffs

CANCommunicator6A::CANCommunicator6A(CANInterface *can, uint8_t bus, CANDiagCalculator *c):
	CANCommunicatorMaster(can,bus,c) {
	state = IDLE;
}

// There is a weird thing here, when I send the cmd before adding the callback id to listen
// There is a lot of chance to miss the answer, this is true when simulation (local loop), 
// don't know in real life. 

void CANCommunicator6A::ExecuteCommand(char *data, uint8_t len, char *response, uint8_t *respLen, uint32_t msTmo) {
	osEvent evt;
	state = IDLE;
	uint8_t currLen = 0;
	bool endCmd = false;
	if(respLen != NULL)
		*respLen = 0;
	else
		return;
	CANDiagCommandHeader *hdrCmd = NULL;
	//DBG("bus %d - calc %p", bus, calc);
	if((bus != 0) && (calc != NULL)) {
		// Flush queue
		FlushQueue();
		// Tell we want receive the event now
		canItf->AddCallBackForId(bus, calc->GetAddrDst(), this);
		// Multiple lines is not handled for the moment
		DBG_MEMDUMP("(6A)ExecuteCommand - Send CMD",data,len);
		//DBG("Sending %04x",calc->GetAddrSrc());
		canItf->Send(bus,calc->GetAddrSrc(),data,len);
		// If no Response and no timeout and no respLen pointer do nothing
		if((msTmo != 0) && (response != NULL) && (respLen != NULL)) {
			// Wait for answer
			while(!endCmd) {
				// TODO: Compute timeouts
				// DBG("GET = %d",msTmo);
				evt = queue.get(msTmo);
				if(evt.status == osEventMessage) {
					CANMessage *msg = (CANMessage*) evt.value.p;
					DBG("(6A)ExecuteCommand - Received Response");
					switch(state) {
						case IDLE:
							if( (msg->data[0] & 0xf0) == 0x00){
								DBG("(6A)ExecuteCommand - Got Short Answer");
								// Record the response data
								*respLen = msg->data[0];
								memcpy(response,msg->data+1,*respLen);
								// Tell we got it all
								endCmd = true;
							} else if(msg->data[0] == 0x10){
								// Record the response data
								*respLen = msg->data[1];
								memcpy(response,msg->data+2,6);
								currLen = 6;
								DBG("(6A)ExecuteCommand - Got Extended Answer First Part (%d/%d) => Send Ack",currLen,*respLen);
								// Get the ACK Command in calculator structure
								calc->FirstCommand(&hdrCmd,CMD_ACK);
								if(hdrCmd){
									// Send the ACK
									canItf->Send(bus,calc->GetAddrSrc(),hdrCmd->cmd,hdrCmd->size);
									state = EXT_CMD;
								} else {
									DBG("(6A)ExecuteCommand - Could Not Find the ACK");
								}
							} else {
								DBG("(6A)ExecuteCommand - Not handled case");
							}
						break;
						case EXT_CMD:
							if((msg->data[0] & 0xf0) == 0x20){
								DBG("(6A)ExecuteCommand - Got Extended Answer (%d) Parts => Send Ack",msg->data[0] & 0x0f);
								// Record the response data
								uint8_t payloadSz = ((*respLen-currLen)<7)?(*respLen-currLen):7;						
								memcpy(response+currLen,msg->data+1,payloadSz);
								currLen += payloadSz;
								if(currLen >= *respLen) {
									DBG("(6A)ExecuteCommand - Extended Answer got completed (%d/%d)",currLen,*respLen);
									state = IDLE;
									endCmd = true;
								}
							} else {
								DBG("(6A)ExecuteCommand - Wrong Extended frame header ... continue");
							}
						break;
					}
					delete(msg);
				} else if (evt.status == osEventTimeout) {
					//PrintActiveThreads();
					DBG("(6A)ExecuteCommand - failed (timeout)");
					// Reset state machine and exit rcv loop
					state = IDLE;
					endCmd = true;
					break;
				} else {
					DBG("(6A)Other type");
				}
			}
		}
		// read queue
		canItf->DelCallBackForId(bus, calc->GetAddrDst());
	}
}

// The SLAVE Stuffs
#define NO_RESP					00
#define SHORT_RESP			01
#define LONG_RESP 			02

CANCommunicatorSim6A::CANCommunicatorSim6A(CANInterface *can, uint8_t bus, CANDiagCalculator *c):
	CANCommunicatorSlave(can,bus,c) {
	state = IDLE;
	iteration = 0;
}

uint8_t CANCommunicatorSim6A::RequestDB(char *cmd, uint8_t clen, char *results, uint8_t *rlen) {
	uint8_t ret = NO_RESP;
	*rlen = 0;
	// It comes without the sz byte but goes out with it
	//DBG_MEMDUMP("RequestDB", cmd, clen);
	if(clen == 2) {
		if( ((cmd[0] == 0x10) && (cmd[1] == 0xC0)) ||
				((cmd[0] == 0x21) && (cmd[1] == 0xB0)) ||
				((cmd[0] == 0x21) && (cmd[1] == 0xC0)) ||
				((cmd[0] == 0x21) && (cmd[1] == 0xCB)) ||
				((cmd[0] == 0x21) && (cmd[1] == 0xC2)) ||
				((cmd[0] == 0x21) && (cmd[1] == 0xC9)) ) {
			DBG("ComSlave: Talking to us 0x10C0");
			  memcpy(results,"__cdefghaimeleshommesgayyz",26);
			//memcpy(results,"__cdefghijklmnopqrstuvwxyz",26);
			results[0] = 25;
			results[1] = cmd[0] + 0x40;
			results[2] = cmd[1];
			*rlen = 26;
		}
	}
	if(clen == 3) {
		if((cmd[0] == 0x03) && (cmd[1] == 0x02) && (cmd[2] == 0x01)) {
			DBG("ComSlave: This is an opening sequence");
		}
	}
	if(*rlen) {
		ret = (*rlen<=7)?SHORT_RESP:LONG_RESP;
	}
	return ret;
}

void CANCommunicatorSim6A::FormatResponse(char *response, uint8_t responseSz, char *payload, uint8_t *payloadSz, uint8_t it) {
	uint8_t respType = (responseSz<=7)?SHORT_RESP:LONG_RESP;
	//DBG("Format this is a %d type", respType);
	if(respType == SHORT_RESP) {
		//DBG("RespSz = %d", responseSz);
		memcpy(payload,response,responseSz);
		*payloadSz = responseSz;
	} else if (respType == LONG_RESP) {
		if(it == 0)
			payload[0] = 0x10;
		else
			payload[0] = 0x20 | (0x0f & it);
		uint8_t l = (((it+1)*7)<=responseSz)?7:responseSz%7;
		memcpy(payload+1,response+(it*7),l);
		//DBG("It = %d | RespSz = %d", it,l);
		*payloadSz = l+1;
	}
}

void CANCommunicatorSim6A::event(int ID, void *data) {
	// Payload is an exchange buffer 
	char payload[7];
	uint8_t respType = 0;
	// Check the callback is well destinated
	if((ID&0x0f) == CAN_CALLBACK){
		CANMessage *msg = (CANMessage*) data;
		uint8_t payloadSz = 0;
		// There is two thing to watch
		//   * keepalive
		//	 * Commands
		//   * Commands Extended
		DBG_MEMDUMP("ComSlave",(const char*)msg->data,msg->len);

		// Check if this is a MAINTAIN STUFF
		if( (msg->data[0] == 0x01) && (msg->data[1] == 0x3E) ) {
			char m[2];
			DBG("ComSlave: Respond to KeepAlive");
			m[0] = 0x01;	// sz
			m[1] = 0x7E;  // data
			canItf->Send(bus,calc->GetAddrSrc(),m,2);
		} else {
			switch(state) {
				case IDLE: {
					// Frame type check
					if(msg->data[0] & 0x0f) {
						iteration = 0;
						DBG("ComSlave: Got short");
						// Get The size and data
						payloadSz = msg->data[0];
						memcpy(payload,msg->data+1,payloadSz);
						// Request the response from the received payload
						respType = RequestDB(payload,payloadSz,response,&rLen);
						if(respType != NO_RESP) {
							// Format the response into payload
							FormatResponse(response,rLen,payload,&payloadSz,iteration);
							// Send the payload
							canItf->Send(bus,calc->GetAddrSrc(),payload,payloadSz);
							if(respType == SHORT_RESP) {
								DBG("ComSlave: Short Resp");
							} else if(respType == LONG_RESP) {
								iteration++;
								DBG("ComSlave: Long Resp");
								state = EXT_CMD;
							}
						}
					} else {
						DBG("ComSlave: Don't know how to deal with long question");
					}
					break; 
				}
				case EXT_CMD: {
					// Here we wait for the ACK but we can get 
					if( (msg->data[0] == 0x30) && (msg->data[1] == 0x00) && (msg->data[2] == 0x0A)) {
						DBG("ComSlave: Got ACK send the EXT part");
						for(uint8_t i = 7; i<rLen; i+=7){
							FormatResponse(response,rLen,payload,&payloadSz,iteration);
							canItf->Send(bus,calc->GetAddrSrc(),payload,payloadSz);
							iteration++;
						}
						rLen = 0;
					}
					state = IDLE;
					break;
				}
			}
		}
	}
}



