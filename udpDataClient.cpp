
#include "udpDataClient.h"
#include "Config.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "udpDataClient.cpp"
#endif
#include "MyDebug.h"

const char* UDP_SERVER_ADDRESS = "217.160.2.145";
const int 	UDP_SERVER_PORT = 6666;

udpDataClient::udpDataClient() {
	DBG("udpDataClient");
	// Socket
	socket.init();
	// Set address EP
	server.set_address(UDP_SERVER_ADDRESS, UDP_SERVER_PORT);
}

int udpDataClient::GetTestIDs(char *data,uint16_t len) {
	DBG("GetTestCanIds");
	char d[1] = {0};
	socket.sendTo(server, d, 1);	
	return socket.receiveFrom(server, data,len);
}


int udpDataClient::GetConfig(uint8_t index, char *data,uint16_t len) {
	DBG("GetConfig");
	char d[2] = {1,0};
	d[1] = index;
	DBG_MEMDUMP("RX getconfig",d,2);
	socket.sendTo(server, d, 2);
	return socket.receiveFrom(server, data,len);
}

void udpDataClient::SendData(char *data, uint16_t len) {
	DBG("SendData");
	data[0] = 2; // Override the frame mode
    socket.sendTo(server, data, len);
}

//void udpDataClient::DoServerRequest()