
#include "tcpDataClient.h"
#include "Config.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "tcpDataClient.cpp"
#endif
#include "MyDebug.h"

const char* SERVER_ADDRESS = "217.160.2.145";
const int 	SERVER_PORT = 6666;

tcpDataClient::tcpDataClient(unsigned char *sp, unsigned int sSz):MyThread("tcpDataClient",sSz,sp), socket() {
	DBG("TCPDataClient");
	while (socket.connect(SERVER_ADDRESS, SERVER_PORT) < 0) {
        DBG("Unable to connect to (%s) on port (%d)\n", SERVER_ADDRESS, SERVER_PORT);
        Thread::wait(1000);
    }
    DBG("Connected to Server at %s\n",SERVER_ADDRESS);
}

bool tcpDataClient::isConnected(void) {
	return socket.is_connected();
}

void tcpDataClient::Main(void) {
	DBG("Running");
    while(running) {
    	DBG("Loop");
    	Thread::wait(1000);
		//DoServerRequest();
    }
}

void tcpDataClient::SendData(char *data, uint16_t len) {
	if(isConnected()){
		//DBG("SendData (%d)",len);
		//char hello[] = "Hello World";
    	//printf("Sending  message to Server : '%s' \n",hello);
    	//socket.send_all(hello, sizeof(hello) - 1);
		socket.send_all(data, len);
	} else {
		DBG("don't SendData");
	}
}

int tcpDataClient::GetConfig(char *data,uint16_t len) {
	if(isConnected()){
		DBG("GetConfig");
		char d[1] = {0};
		socket.send_all(d, 1);
		return socket.receive(data,len);
	} else{
		DBG("don't GetConfig");
	}
}

//void tcpDataClient::DoServerRequest()