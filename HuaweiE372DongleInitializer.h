
#ifndef HUAWEI372_DONGLE_INITIALIZER_H
#define HUAWEI372_DONGLE_INITIALIZER_H

#include "WANDongleInitializer.h"

#define WAN_DONGLE_TYPE_HUAWEI_E372  1

// For the moment we have only one key
class HuaweiE372DongleInitializer: public WANDongleInitializer {
public:
    HuaweiE372DongleInitializer(USBHost *h);
    virtual uint16_t getMSDVid() { return 0x12D1; }
    virtual uint16_t getMSDPid() { return 0x1505; }
    virtual uint16_t getSerialVid() { return 0x12D1; }
    virtual uint16_t getSerialPid() { return 0x14ac; }
    virtual bool switchMode(USBDeviceConnected* pDev) ;
    virtual USBEndpoint* getEp(USBDeviceConnected* pDev, int serialPortNumber, bool tx);
    virtual int getSerialPortCount() { return 4; }
    virtual void setVidPid(uint16_t vid, uint16_t pid) ;
    virtual bool parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol); //Must return true if the interface should be parsed
    virtual bool useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir) ; //Must return true if the endpoint will be used
    virtual int getType() { return WAN_DONGLE_TYPE_HUAWEI_E372; }
    virtual uint8_t getSerialIntf(int index) { return index; }
private:
    bool m_hasSwitched;
    int m_currentSerialIntf;
    int m_endpointsToFetch;
};

#endif  // HUAWEI372_DONGLE_INITIALIZER_H