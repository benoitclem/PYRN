
#ifndef PPPIPINTERFACE_H_
#define PPPIPINTERFACE_H_

#include "core/fwk.h"
#include "lwip/sio.h"
#include "USBSerialStream.h"

class PPPIPInterface {
private:
    char ipAddress[16];
    
    USBSerialStream* pppStream; //Serial stream
    
    int cleanupLink();
    int pppSession;
    bool ipInitiated;
    bool pppInitiated;
    bool connected;
public:
    PPPIPInterface(USBSerialStream* pStream);
    virtual ~PPPIPInterface();
    
    void stackInits(const char* user, const char* pw);
    int dial(void);
    int escape(void);
    
    int connect(const char* user, const char* pw);
    int disconnect();
    
    void setConnected(bool val);
    bool isConnected(void);
    
    // IP stuffs
    char *getIPAddress(void);
    void setIPAddress(char *ip);
    
    // PPP implementaion
    friend u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len);
    friend u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len);
    friend void sio_read_abort(sio_fd_t fd);

    // CallBacks
    static void tcpipInitDoneCb(void *arg);
    static void linkStatusCb(void *ctx, int errCode, void *arg);
};

#endif /* PPPIPINTERFACE_H_ */
