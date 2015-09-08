
#include "sd.h"

#define __DEBUG__ 5
#ifndef __MODULE__
#define __MODULE__ "sd.cpp"
#endif
#include "MyDebug.h"
 
//#define DEBUG
#define SD_COMMAND_TIMEOUT 5000
char bufftmp[512]; 
char cbbufftmp[512];
 
sdCard::sdCard(PinName mosi, PinName miso, PinName sclk, PinName cs) :
  _spi(mosi, miso, sclk), _cs(cs) {
      _cs = 1; 
}
 
#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)
 
// Types
//  - v1.x Standard Capacity
//  - v2.x Standard Capacity
//  - v2.x High Capacity
//  - Not recognised as an SD Card
 
#define SDCARD_FAIL 0
#define SDCARD_V1   1
#define SDCARD_V2   2
#define SDCARD_V2HC 3
 
int sdCard::initialise_card() {
    // Set to 100kHz for initialisation, and clock card with cs = 1
    _spi.frequency(100000); 
    _cs = 1;
    for(int i=0; i<16; i++) {   
        _spi.write(0xFF);
    }
 
    // send CMD0, should return with all zeros except IDLE STATE set (bit 0)
    if(_cmd(0, 0) != R1_IDLE_STATE) { 
        ERR("No disk, or could not put SD card in to SPI idle state\n");
        return SDCARD_FAIL;
    }
 
    // send CMD8 to determine whther it is ver 2.x
    int r = _cmd8();
    if(r == R1_IDLE_STATE) {
        return initialise_card_v2();
    } else if(r == (R1_IDLE_STATE | R1_ILLEGAL_COMMAND)) {
        return initialise_card_v1();
    } else {
        ERR("Not in idle state after sending CMD8 (not an SD card?)\n");
        return SDCARD_FAIL;
    }
}
 
int sdCard::initialise_card_v1() {
    for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
        _cmd(55, 0); 
        if(_cmd(41, 0) == 0) { 
            cdv = 512;
            #ifdef DEBUG 
            DBG("\n\rInit: SDCARD_V1\n\r");
            #endif
            return SDCARD_V1;
        }
    }
 
    ERR("Timeout waiting for v1.x card\n");
    return SDCARD_FAIL;
}
 
int sdCard::initialise_card_v2() {
    
    for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
        wait_ms(50);
        _cmd58();
        _cmd(55, 0); 
        if(_cmd(41, 0x40000000) == 0) { 
            _cmd58();
            DBG("\n\rInit: SDCARD_V2\n\r");
            cdv = 1;
            return SDCARD_V2;
        }
    }
 
    ERR("Timeout waiting for v2.x card\n");
    return SDCARD_FAIL;
}
 
int sdCard::disk_initialize() {
 
    int i = initialise_card();
    #ifdef DEBUG 
    printf("init card = %d\n", i);
    #endif
    _sectors = _sd_sectors();
 
    // Set block length to 512 (CMD16)
    if(_cmd(16, 512) != 0) {
        ERR("Set 512-byte block timed out\n");
        return 1;
    }
        
    _spi.frequency(1000000); // Set to 1MHz for data transfer
    return 0;
}
 
int sdCard::disk_write(const char *buffer, int block_number) {
	if(access.trylock()){   
	    // set write address for single block (CMD24)
	    if(_cmd(24, block_number * cdv) != 0) {
	        return 1;
   		}
 
    	// send the data block
    	_write(buffer, 512);
    	access.unlock();
    	return 0;
    }
    return 1;
}
 
int sdCard::disk_read(char *buffer, int block_number) {   
	if(access.trylock()){     
	    // set read address for single block (CMD17)
	    if(_cmd(17, block_number * cdv) != 0) {
	        return 1;
   		 }
    
    	// receive the data
    	_read(buffer, 512);
    	access.unlock();
    	return 0;
    } 
    return 1;
}
 
int sdCard::disk_sectors() { return _sectors; }
 
// PRIVATE FUNCTIONS
 
int sdCard::_cmd(int cmd, int arg) {
    _cs = 0; 
 
    // send a command
    _spi.write(0x40 | cmd);
    _spi.write(arg >> 24);
    _spi.write(arg >> 16);
    _spi.write(arg >> 8);
    _spi.write(arg >> 0);
    _spi.write(0x95);
 
    // wait for the repsonse (response[7] == 0)
    for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
        int response = _spi.write(0xFF);
        if(!(response & 0x80)) {
            _cs = 1;
            _spi.write(0xFF);
            return response;
        }
    }
    _cs = 1;
    _spi.write(0xFF);
    return -1; // timeout
}

int sdCard::_cmdx(int cmd, int arg) {
    _cs = 0; 
 
    // send a command
    _spi.write(0x40 | cmd);
    _spi.write(arg >> 24);
    _spi.write(arg >> 16);
    _spi.write(arg >> 8);
    _spi.write(arg >> 0);
    _spi.write(0x95);
 
    // wait for the repsonse (response[7] == 0)
    for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
        int response = _spi.write(0xFF);
        if(!(response & 0x80)) {
            return response;
        }
    }
    _cs = 1;
    _spi.write(0xFF);
    return -1; // timeout
}
 
 
int sdCard::_cmd58() {
    _cs = 0; 
    int arg = 0;
    
    // send a command
    _spi.write(0x40 | 58);
    _spi.write(arg >> 24);
    _spi.write(arg >> 16);
    _spi.write(arg >> 8);
    _spi.write(arg >> 0);
    _spi.write(0x95);
 
    // wait for the repsonse (response[7] == 0)
    for(int i=0; i<SD_COMMAND_TIMEOUT; i++) {
        int response = _spi.write(0xFF);
        if(!(response & 0x80)) {
            int ocr = _spi.write(0xFF) << 24;
            ocr |= _spi.write(0xFF) << 16;
            ocr |= _spi.write(0xFF) << 8;
            ocr |= _spi.write(0xFF) << 0;
//            printf("OCR = 0x%08X\n", ocr);
            _cs = 1;
            _spi.write(0xFF);
            return response;
        }
    }
    _cs = 1;
    _spi.write(0xFF);
    return -1; // timeout
}
 
int sdCard::_cmd8() {
    _cs = 0; 
    
    // send a command
    _spi.write(0x40 | 8); // CMD8
    _spi.write(0x00);     // reserved
    _spi.write(0x00);     // reserved
    _spi.write(0x01);     // 3.3v
    _spi.write(0xAA);     // check pattern
    _spi.write(0x87);     // crc
 
    // wait for the repsonse (response[7] == 0)
    for(int i=0; i<SD_COMMAND_TIMEOUT * 1000; i++) {
        char response[5];
        response[0] = _spi.write(0xFF);
        if(!(response[0] & 0x80)) {
                for(int j=1; j<5; j++) {
                    response[i] = _spi.write(0xFF);
                }
                _cs = 1;
                _spi.write(0xFF);
                return response[0];
        }
    }
    _cs = 1;
    _spi.write(0xFF);
    return -1; // timeout
}
 
int sdCard::_read(char *buffer, int length) {
    _cs = 0;
 
    // read until start byte (0xFF)
    while(_spi.write(0xFF) != 0xFE);
 
    // read data
    for(int i=0; i<length; i++) {
        buffer[i] = _spi.write(0xFF);
    }
    _spi.write(0xFF); // checksum
    _spi.write(0xFF);
 
    _cs = 1;    
    _spi.write(0xFF);
    return 0;
}
 
int sdCard::_write(const char *buffer, int length) {
    _cs = 0;
    
    // indicate start of block
    _spi.write(0xFE);
    
    // write the data
    for(int i=0; i<length; i++) {
        _spi.write(buffer[i]);
    }
    
    // write the checksum
    _spi.write(0xFF); 
    _spi.write(0xFF);
 
    // check the repsonse token
    if((_spi.write(0xFF) & 0x1F) != 0x05) {
        _cs = 1;
        _spi.write(0xFF);        
        return 1;
    }
 
    // wait for write to finish
    while(_spi.write(0xFF) == 0);
 
    _cs = 1; 
    _spi.write(0xFF);
    return 0;
}
 
static int ext_bits(char *data, int msb, int lsb) {
    int bits = 0;
    int size = 1 + msb - lsb; 
    for(int i=0; i<size; i++) {
        int position = lsb + i;
        int byte = 15 - (position >> 3);
        int bit = position & 0x7;
        int value = (data[byte] >> bit) & 1;
        bits |= value << i;
    }
    return bits;
}
 
int sdCard::_sd_sectors() {
 
    int c_size, c_size_mult, read_bl_len;
    int block_len, mult, blocknr, capacity;       
    int blocks, hc_c_size;
     
    // CMD9, Response R2 (R1 byte + 16-byte block read)
    if(_cmdx(9, 0) != 0) {
        ERR("Didn't get a response from the disk\n");
        return 0;
    }
    
    char csd[16];    
    if(_read(csd, 16) != 0) {
        ERR("Couldn't read csd response from disk\n");
        return 0;
    }
 
    // csd_structure : csd[127:126]
    // c_size        : csd[73:62]
    // c_size_mult   : csd[49:47]
    // read_bl_len   : csd[83:80] - the *maximum* read block length
   
    int csd_structure = ext_bits(csd, 127, 126);
    
    DBG("\n\rCSD_STRUCT = %d\n", csd_structure);
     
    switch (csd_structure){
     case 0:
      cdv = 512;
      c_size = ext_bits(csd, 73, 62);
      c_size_mult = ext_bits(csd, 49, 47);
      read_bl_len = ext_bits(csd, 83, 80);
     
      block_len = 1 << read_bl_len;
      mult = 1 << (c_size_mult + 2);
      blocknr = (c_size + 1) * mult;
      capacity = blocknr * block_len;
      blocks = capacity / 512;
      DBG("\n\rSDCard\n\rc_size: %.4X \n\rcapacity: %.ld \n\rsectors: %d\n\r", c_size, capacity, blocks);
      break;
    
     case 1:
     {
      cdv = 1;
      hc_c_size = ext_bits(csd, 63, 48);
      int hc_read_bl_len = ext_bits(csd, 83, 80);
      blocks = (hc_c_size+1)*1024;
      #ifdef DEBUG 
      uint64_t hc_capacity = hc_c_size+1;   
      DBG("\n\rSDHC Card \n\rhc_c_size: %.4X \n\rcapacity: %.lld \n\rsectors: %d\n\r", hc_c_size, hc_capacity*512*1024, blocks);
      #endif
      break;
     }
   
    default:    
       ERR("This disk tastes funny! I only know about type 0 CSD structures\n");
     return 0;
    };
 return blocks;
}

sdStorage::sdStorage(PinName mosi, PinName miso, PinName sclk, PinName cs, int iSectStart, int nSects) {
	if(sd == NULL) {
		DBG("First sdStorage instanciation");
		sd = new sdCard(mosi, miso, sclk, cs);
		sd->disk_initialize();
	}
	int maxSectors = sd->disk_sectors();
	if(nSects < maxSectors) {
		DBG("OK, create the circular buffer");
		iSectorStart = iSectStart;
		nSectors = nSects;
		//usedSectors = 0;
		//rSectorIndex = 0;
		//wSectorIndex = 0;
	} else {
		ERR("The number of sector you asked is bigger that the sd card sector number");
	}
}

int sdStorage::Read(char *buffer, int index, int length, int offset) {
	int err = 1;
	int nRead = 0;
	int currSectorIndex = iSectorStart+index;
	// Coarse Read size
	if((((nSectors-index)*512)-offset)<length) {
		WARN("SD-Read: Try to read %d but max is %d",length,((nSectors-index)*512)-offset);
		length = (((nSectors-index)*512)-offset);
	}
	DBG("SD-Read:before lock");
	int i = 0;
	int nCpy = 0;
	while(i<length) {
		if((offset!=0) && (i == 0)){
			nCpy = 512 - offset;
		} else {
			nCpy = ((length-i)>512)?512:(length-i);
		}
		err = sd->disk_read(bufftmp,currSectorIndex);
		if(err) {
			ERR("SD-Read: Something goes wrong");
		} else {
			DBG("SD-Read: GOOD %d/%d %d",i+nCpy,length,currSectorIndex);
		}
		memcpy(buffer+i,bufftmp+offset,nCpy);
		if(offset)
			offset = 0;
		i+=nCpy;
		currSectorIndex++;
	}
	return err;
}

int sdStorage::Write(char *buffer, int index, int length, int offset) {
	int err = 1;
	int nWrite = 0;
	int currSectorIndex = index;
	// Coarse Write size t
	if((((nSectors-index)*512)-offset)<length) {
		WARN("SD-Write: Try to read %d but max is %d",length,((nSectors-index)*512));
		length = ((nSectors-index)*512);
	}
	DBG("SD-Write: before lock");
	int i = 0;
	int nCpy = 0;
	while(i<length) {
		if((offset!=0) && (i == 0)){
			DBG("SD-Write: offset %d",offset);
			// read the part that exists before offset
			sd->disk_read(bufftmp,currSectorIndex);
			nCpy = (length>(512-offset))?(512-offset):length;
		} else {
			memset(bufftmp,0,512);
			nCpy = ((length-i)>512)?512:(length-i);
		}
		memcpy(bufftmp+offset,buffer+i,nCpy);
		if(offset)
			offset = 0;
		err = sd->disk_write(bufftmp,currSectorIndex);
		if(err) {
			ERR("SD-Write: Something goes wrong");
			break;
		} else {
			DBG("SD-Write: GOOD %d/%d %d",i+nCpy,length,currSectorIndex);
		}
		i+=nCpy;
		currSectorIndex++;
	}
	return err;
}

int sdStorage::Clear(int index) {
	memset(bufftmp,0,512);
	int err = sd->disk_write(bufftmp,index);
	if(err) {
		ERR("SD-Clear: Something goes wrong");
	} else {
		DBG("SD-Clear: GOOD %d",index);
	}
	return err;
}

/*
sdCircBuff::sdCircBuff(PinName mosi, PinName miso, PinName sclk, PinName cs, int iSectStart, int nSects): 
	sdStorage(mosi, miso, sclk, cs, iSectStart, nSects) {
	// Trash first sector
	Clear(0);
	// Initiate the pointers
	pointers.w = 0;
	pointers.sw = 0;
	pointers.r = 0;
	pointers.sr = 0;
	pointers.sz = 0;
}
	
// /!\ This is not thread safe /!\ .
sdCircBuff::Put(char *buffer, int index, int length){
	int offset = 0;
	int nPut = 0;
	if(pointers.sw != 0) {
		// Need to read the data in sector
		sd.read(cbbufftmp,pointers.w,pointers.sw+1);
		offset = pointers.sw;
	}
	// Insert the size
	if((512-pointers.sw) < sizeof(int)) {
		// Size gonna be splitted at one sector boundary
		cbbufftmp
		memcpy(cbbufftmp+pointer.sw,(char*)&length,((512-pointers.sw) < sizeof(int))?512-pointers.sw:4);
		sd.write(((char*)&length)+
	} else {
	}
	memcpy(cbbufftmp+pointers.sw,(char*)&length,sizeof(int));
	// put the size of block that will follow
	sd.write((char*)&length,pointers.w,sizeof(int));
	while(nPut<length){

}
*/

/*
int sdCircBuff::ComputeChecksum(int p1, int p2, int p3, int p4) {
	return p1+p2+p3+p4;
}

char sdCircBuff::CheckChecksum(int p1, int p2, int p3, int p4, char cs) {
	return cs == ComputeCheckSum(p1,p2,p3,p4);
}
*/

/* En partique ca ne peu pas marcher
int sdStorage::BCRead(char *buffer, int length) {
	int err = 1;
	DBG("SD-Read: length = %d",length);
	if(rSectorIndex != wSectorIndex) {
		if(SectorAccess.trylock()){
			if(usedSectors) {
				for(int i = 0; i<length; i+=512) {
					DBG("SD-Read-B: rSector = %d | wSector = %d | used = %d",rSectorIndex,wSectorIndex, usedSectors);
					err = sd->disk_read(buffer+i,rSectorIndex);
					if(err) {
						ERR("SD-Write: Something goes wrong");
					}
					rSectorIndex++;
					if(rSectorIndex >= nSectors)
						rSectorIndex = 0;
					usedSectors--;
					DBG("SD-Read-A: rSector = %d | wSector = %d | used = %d",rSectorIndex,wSectorIndex, usedSectors);
				}
			}
			SectorAccess.unlock();
		}
	}
	return err;
}
		
int sdStorage::BCWrite(char *buffer, int length) {
	int err = 1;
	int wSectTempIndex = wSectorIndex;
	int usedTempSectors = usedSectors;
	int nSectorToWrite = (length/512)+(length%512)?1:0;
	DBG("SD-Write: length = %d, nSectorToWrite = %d",length, nSectorToWrite);
	if(nSectorToWrite) {
		if(SectorAccess.trylock()){
			for(int i = 0; i< length; i+=512) {
				DBG("SD-Write-B: rSector = %d | wSector = %d | used = %d",rSectorIndex,wSectTempIndex, usedTempSectors);
				err = sd->disk_write(buffer+i,wSectTempIndex);
				if(err) {
					ERR("SD-Write: Something goes wrong");
					break;
				}
				wSectTempIndex++;
				if(wSectTempIndex >= nSectors)
					wSectTempIndex = 0;
				usedTempSectors++; 
				DBG("SD-Write-A: rSector = %d | wSector = %d | used = %d",rSectorIndex,wSectTempIndex, usedTempSectors);
			}
			if(err == 0) {
				wSectorIndex = wSectTempIndex;
				usedSectors = usedTempSectors;
			}
			SectorAccess.unlock();
		}
	}
	return err;
}
		
*/		