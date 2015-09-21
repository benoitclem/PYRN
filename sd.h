
#ifndef SD_H
#define	SD_H

#include "mbed.h"
#include "rtos.h"

class sdCard{
public:
    sdCard(PinName mosi, PinName miso, PinName sclk, PinName cs);
    virtual int disk_initialize();
    virtual int disk_write(const char *buffer, int block_number);
    virtual int disk_read(char *buffer, int block_number);    
    virtual int disk_sectors();
 
protected:
	Mutex access;
    int _cmd(int cmd, int arg);
    int _cmdx(int cmd, int arg);
    int _cmd8();
    int _cmd58();
    int initialise_card();
    int initialise_card_v1();
    int initialise_card_v2();
    
    int _read(char *buffer, int length);
    int _write(const char *buffer, int length);
    int _sd_sectors();
    int _sectors;
    
    SPI _spi;
    DigitalOut _cs; 
    int cdv;    
};

#include "storageBase.h"

static sdCard *sd = NULL;
static Mutex *access = NULL;

class sdStorage: public storage{
	public:
		sdStorage(PinName mosi, PinName miso, PinName sclk, PinName cs, int iSectStart, int nSects);
		int Read(char *buffer, int index, int length, int offset = 0);
		int Write(char *buffer, int index, int length, int offset = 0);
		int Clear(int index);
	protected:

		int iSectorStart;
		int	nSectors;
};

// The CircBuff is a multiple thread access so add mutex
class sdCircBuff: public sdStorage {
//class sdCircBuff: public circBuff{
	public:
		sdCircBuff(PinName mosi, PinName miso, PinName sclk, PinName cs, int iSectStart, int nSects);
		int Put(char *buffer, int length);
		int Get(char *buffer, int length);
		int Probe();
		//int ComputeChecksum(int p1, int p2, int p3, int p4);
		//bool CheckChecksum(int p1, int p2, int p3, int p4, int cs);
	protected:
		int size;
		int current;
		int pRead;
		int pWrite;
};
#endif