
#include "HuaweiE372DongleInitializer.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "MyThread.cpp"
#endif
#include "MyDebug.h"

#define VENDSPEC_CLASS  0xFF

//Huawei E372 (Vodafone)

// switch string => "55 53 42 43 00 00 00 00 00 00 00 00 00 00 00 11 06 00 00 00 00 00 00 00 00 00 00 00 00 00 00"
static uint8_t huawei_E372_switch_packet[] = {
    0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x11, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};

HuaweiE372DongleInitializer::HuaweiE372DongleInitializer(USBHost *h): WANDongleInitializer(h) {
        
}

bool HuaweiE372DongleInitializer::switchMode(USBDeviceConnected* pDev) {
    for (int i = 0; i < pDev->getNbIntf(); i++) {
        if (pDev->getInterface(i)->intf_class == MSD_CLASS) {
            USBEndpoint* pEp = pDev->getEndpoint(i, BULK_ENDPOINT, OUT);
            if ( pEp != NULL )  {
                USB_DBG("MSD descriptor found on device %p, intf %d, will now try to switch into serial mode", (void *)pDev, i);
                m_pHost->bulkWrite(pDev, pEp, huawei_E372_switch_packet, 31);
                return true;
            }
        }  
    }
    return false;
}

USBEndpoint* HuaweiE372DongleInitializer::getEp(USBDeviceConnected* pDev, int serialPortNumber, bool tx) {
    return pDev->getEndpoint(serialPortNumber, BULK_ENDPOINT, tx ? OUT : IN, 0);
}

void HuaweiE372DongleInitializer::setVidPid(uint16_t vid, uint16_t pid) {
    if( (vid == getSerialVid()) && (pid == getSerialPid()) ) {
      m_hasSwitched = true;
      m_currentSerialIntf = 0;
      m_endpointsToFetch = 4*2;
    } else {
      m_hasSwitched = false;
      m_endpointsToFetch = 1;
    }
}
 
bool HuaweiE372DongleInitializer::parseInterface(uint8_t intf_nb, uint8_t intf_class, uint8_t intf_subclass, uint8_t intf_protocol) {
    if( m_hasSwitched ) {
        USB_DBG("Interface #%d; Class:%02x; SubClass:%02x; Protocol:%02x", intf_nb, intf_class, intf_subclass, intf_protocol);
        if( intf_class == VENDSPEC_CLASS ) {
            // The first 4 Interfaces are parsable.
            if( m_currentSerialIntf <4 ) {
                m_currentSerialIntf++;
                return true;
            }
            m_currentSerialIntf++;
        }
    } else {
        // The first 2 Interface are parsable.
        if( ((intf_nb == 0) || (intf_nb == 1)) && (intf_class == MSD_CLASS) ) {
            return true;
        }
    }
    return false;
}

bool HuaweiE372DongleInitializer::useEndpoint(uint8_t intf_nb, ENDPOINT_TYPE type, ENDPOINT_DIRECTION dir) {//Must return true if the endpoint will be used
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