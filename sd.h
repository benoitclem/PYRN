
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

class sdStorage{
	public:
		sdStorage(PinName mosi, PinName miso, PinName sclk, PinName cs, int nSects);
		int Read(char *buffer, int length);
		int Write(char *buffer, int length);
				
	protected:
		Mutex SectorAccess;
		sdCard 	sd;
		int	nSectors;
		int usedSectors;
		int	rSectorIndex;
		int	wSectorIndex;
};

#endif