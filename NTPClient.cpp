
#include "mbed.h"
#include "NTPClient.h"
#include "UDPSocket.h"

#define __DEBUG__ 0
#ifndef __MODULE__
#define __MODULE__ "CANDiagCalculator.cpp"
#endif
#include "MyDebug.h"

#define NTP_PORT 123
#define NTP_CLIENT_PORT 0 //Random port
#define NTP_TIMESTAMP_DELTA 2208988800ull //Diff btw a UNIX timestamp (Starting Jan, 1st 1970) and a NTP timestamp (Starting Jan, 1st 1900)

NTPClient::NTPClient() : m_sock() { }

NTPResult NTPClient::setTime(const char* host, uint16_t port, uint32_t timeout) {
#ifdef __DEBUG__
	time_t ctTime;
	ctTime = time(NULL);
	DBG("Time is set to (UTC): %s", ctime(&ctTime));
#endif

	//Create & bind socket
	DBG("Binding socket");
	m_sock.bind(0); //Bind to a random port

	m_sock.set_blocking(false, timeout); //Set not blocking

	struct NTPPacket pkt;

	//Now ping the server and wait for response
	DBG("Ping");
	//Prepare NTP Packet:
	pkt.li = 0; //Leap Indicator : No warning
	pkt.vn = 4; //Version Number : 4
	pkt.mode = 3; //Client mode
	pkt.stratum = 0; //Not relevant here
	pkt.poll = 0; //Not significant as well
	pkt.precision = 0; //Neither this one is

	pkt.rootDelay = 0; //Or this one
	pkt.rootDispersion = 0; //Or that one
	pkt.refId = 0; //...

	pkt.refTm_s = 0;
	pkt.origTm_s = 0;
	pkt.rxTm_s = 0;
	pkt.txTm_s = htonl( NTP_TIMESTAMP_DELTA + time(NULL) ); //WARN: We are in LE format, network byte order is BE

	pkt.refTm_f = pkt.origTm_f = pkt.rxTm_f = pkt.txTm_f = 0;

	Endpoint outEndpoint;

	if( outEndpoint.set_address(host, port) < 0) {
		m_sock.close();
		return NTP_DNS;
	}

	//Set timeout, non-blocking and wait using select
	int ret = m_sock.sendTo( outEndpoint, (char*)&pkt, sizeof(NTPPacket) );
	if (ret < 0 ) {
		ERR("Could not send packet");
		m_sock.close();
		return NTP_CONN;
	}

	//Read response
	Endpoint inEndpoint;

	DBG("Pong");
	do {
		 //FIXME need a DNS Resolver to actually compare the incoming address with the DNS name
		ret = m_sock.receiveFrom( inEndpoint, (char*)&pkt, sizeof(NTPPacket) );
		if(ret < 0) {
			ERR("Could not receive packet");
			m_sock.close();
			return NTP_CONN;
		}
	} while( strcmp(outEndpoint.get_address(), inEndpoint.get_address()) != 0 );

	if(ret < sizeof(NTPPacket)) {//TODO: Accept chunks
		ERR("Receive packet size does not match");
		m_sock.close();
		return NTP_PRTCL;
	}

	if( pkt.stratum == 0) {//Kiss of death message : Not good !
		ERR("Kissed to death!");
		m_sock.close();
		return NTP_PRTCL;
	}

	//Correct Endianness
	pkt.refTm_s = ntohl( pkt.refTm_s );
	pkt.refTm_f = ntohl( pkt.refTm_f );
	pkt.origTm_s = ntohl( pkt.origTm_s );
	pkt.origTm_f = ntohl( pkt.origTm_f );
	pkt.rxTm_s = ntohl( pkt.rxTm_s );
	pkt.rxTm_f = ntohl( pkt.rxTm_f );
	pkt.txTm_s = ntohl( pkt.txTm_s );
	pkt.txTm_f = ntohl( pkt.txTm_f );

	//Compute offset, see RFC 4330 p.13
	uint32_t destTm_s = (NTP_TIMESTAMP_DELTA + time(NULL));
	int64_t offset = ( (int64_t)( pkt.rxTm_s - pkt.origTm_s ) + (int64_t) ( pkt.txTm_s - destTm_s ) ) / 2; //Avoid overflow
	DBG("Sent @%ul", pkt.txTm_s);
	DBG("Offset: %lld", offset);
	//Set time accordingly
	set_time( time(NULL) + offset );

#ifdef __DEBUG__
	ctTime = time(NULL);
	DBG("Time is now (UTC): %s", ctime(&ctTime));
#endif

	m_sock.close();

	return NTP_OK;
}


