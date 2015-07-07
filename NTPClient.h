
#ifndef NTPCLIENT_H_
#define NTPCLIENT_H_

#include "UDPSocket.h"

#define NTP_DEFAULT_PORT 123
#define NTP_DEFAULT_TIMEOUT 4000

///NTP client results
enum NTPResult {
	NTP_DNS, ///<Could not resolve name
	NTP_PRTCL, ///<Protocol error
	NTP_TIMEOUT, ///<Connection timeout
	NTP_CONN, ///<Connection error
	NTP_OK = 0, ///<Success
};

/** NTP Client to update the mbed's RTC using a remote time server
*
*/
class NTPClient {
public:
	NTPClient();
	NTPResult setTime(const char* host, uint16_t port = NTP_DEFAULT_PORT, uint32_t timeout = NTP_DEFAULT_TIMEOUT);

private:
	struct NTPPacket { //See RFC 4330 for Simple NTP
		//WARN: We are in LE! Network is BE!
		//LSb first
		unsigned mode : 3;
		unsigned vn : 3;
		unsigned li : 2;

		uint8_t stratum;
		uint8_t poll;
		uint8_t precision;
		//32 bits header

		uint32_t rootDelay;
		uint32_t rootDispersion;
		uint32_t refId;

		uint32_t refTm_s;
		uint32_t refTm_f;
		uint32_t origTm_s;
		uint32_t origTm_f;
		uint32_t rxTm_s;
		uint32_t rxTm_f;
		uint32_t txTm_s;
		uint32_t txTm_f;
	} __attribute__ ((packed));

	UDPSocket m_sock;
};

#endif /* NTPCLIENT_H_ */

