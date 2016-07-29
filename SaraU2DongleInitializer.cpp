
#include "SaraU2DongleInitializer.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "SaraU2DongleInitializer.cpp"
#endif
#include "MyDebug.h"

#define UBX_SERIALCOUNT 7 
#define VENDSPEC_CLASS  0xFF

//Huawei E372 (Vodafone)


SaraU2DongleInitializer::SaraU2DongleInitializer(USBHost *h): WANDongleInitializer(h) {
    m_initedSerialIntf = 0;
}

bool SaraU2DongleInitializer::switchMode(USBDeviceConnected* pDev) {
    USB_DBG("Nothing to switch... just wait");
    return false;
}

USBEndpoint* SaraU2DongleInitializer::getEp(USBDeviceConnected* pDev, int serialPortNumber, bool tx) {
    return pDev->getEndpoint(m_serialIntfMap[serialPortNumber], BULK_ENDPOINT, tx ? OUT : IN, 0);
}

void SaraU2DongleInitializer::setVidPid(uint16_t vid, uint16_t pid) {
    if( (vid == getSerialVid()) && (pid == getSerialPid()) ) {
      m_hasSwitched = true;
      m_currentSerialIntf = 0;
      m_endpointsToFetch = UBX_SERIALCOUNT*2;
    } else {
      m_hasSwitched = false;
      m_endpointsToFetch = 1;
    }
}
 
bool SaraU2DongleInitializer::parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol) {
    if( m_hasSwitched ) {
        DBG("Interface #%d; Class:%02x; SubClass:%02x; Protocol:%02x", intf_nb, intf_class, intf_subclass, intf_protocol);
        if( intf_class == 0x0A ) {
            if( (m_currentSerialIntf == 0) || (m_currentSerialIntf == 1) ) {
                m_serialIntfMap[m_currentSerialIntf++] = intf_nb;
                DBG("chier %d",m_currentSerialIntf);
                m_initedSerialIntf++;
                return true;
            }
            m_currentSerialIntf++;
        }
    } else {
        if( (intf_nb == 0) && (intf_class == MSD_CLASS) ) {
            return true;
        }
    }
    return false;
}

bool SaraU2DongleInitializer::useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir) {//Must return true if the endpoint will be used
    if( m_hasSwitched ) {
        USB_DBG("USBEndpoint on Inteface #%d; Type:%d; Direction:%d", intf_nb, type, dir);
        if( (type == BULK_ENDPOINT) && m_endpointsToFetch ) {
            m_endpointsToFetch--;
            return true;
        }
    } else {
        if( (type == BULK_ENDPOINT) && (dir == OUT) && m_endpointsToFetch ) {
            m_endpointsToFetch--;
            return true;
        }
    }
    return false;
}