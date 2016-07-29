# This file was automagically generated by mbed.org. For more information,  see
# http://mbed.org/handbook/Exporting-to-GCC-ARM-Embedded

GCC_BIN =  /Users/clemi/Downloads/gcc-arm-none-eabi/bin/
PROJECT = PYRN
OBJECTS = ./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3/TOOLCHAIN_GCC/SVC_Table.o ./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3/TOOLCHAIN_GCC/HAL_CM3.o ./mbed-src/targets/cmsis/TARGET_NXP/TARGET_LPC176X/TOOLCHAIN_GCC_ARM/startup_LPC17xx.o ./lwip/core/mem.o ./lwip/core/tcp.o ./lwip/core/netif.o ./lwip/core/tcp_in.o ./lwip/core/dhcp.o ./lwip/core/memp.o ./lwip/core/tcp_out.o ./lwip/core/udp.o ./lwip/core/def.o ./lwip/core/stats.o ./lwip/core/dns.o ./lwip/core/raw.o ./lwip/core/timers.o ./lwip/core/pbuf.o ./lwip/core/init.o ./lwip/core/ipv4/igmp.o ./lwip/core/ipv4/ip_frag.o ./lwip/core/ipv4/autoip.o ./lwip/core/ipv4/inet.o ./lwip/core/ipv4/icmp.o ./lwip/core/ipv4/ip_addr.o ./lwip/core/ipv4/inet_chksum.o ./lwip/core/ipv4/ip.o ./lwip/core/snmp/msg_in.o ./lwip/core/snmp/asn1_enc.o ./lwip/core/snmp/mib_structs.o ./lwip/core/snmp/asn1_dec.o ./lwip/core/snmp/mib2.o ./lwip/core/snmp/msg_out.o ./lwip/api/netifapi.o ./lwip/api/sockets.o ./lwip/api/netbuf.o ./lwip/api/netdb.o ./lwip/api/api_lib.o ./lwip/api/err.o ./lwip/api/api_msg.o ./lwip/api/tcpip.o ./lwip/netif/etharp.o ./lwip/netif/slipif.o ./lwip/netif/ethernetif.o ./lwip/netif/ppp/auth.o ./lwip/netif/ppp/pap.o ./lwip/netif/ppp/randm.o ./lwip/netif/ppp/md5.o ./lwip/netif/ppp/lcp.o ./lwip/netif/ppp/magic.o ./lwip/netif/ppp/chap.o ./lwip/netif/ppp/ppp.o ./lwip/netif/ppp/ppp_oe.o ./lwip/netif/ppp/ipcp.o ./lwip/netif/ppp/vj.o ./lwip/netif/ppp/fsm.o ./lwip/netif/ppp/chpms.o ./lwip-sys/arch/sys_arch.o ./lwip-sys/arch/checksum.o ./lwip-sys/arch/memcpy.o ./MODSERIAL/ChangeLog.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Semaphore.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Event.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_List.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Mutex.o ./mbed-rtos/rtx/TARGET_CORTEX_M/HAL_CM.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Task.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_CMSIS.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_System.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Time.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_MemBox.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Robin.o ./mbed-rtos/rtx/TARGET_CORTEX_M/RTX_Conf_CM.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Mailbox.o ./mbed-src/common/semihost_api.o ./mbed-src/common/gpio.o ./mbed-src/common/assert.o ./mbed-src/common/error.o ./mbed-src/common/wait_api.o ./mbed-src/common/rtc_time.o ./mbed-src/common/us_ticker_api.o ./mbed-src/common/mbed_interface.o ./mbed-src/common/pinmap_common.o ./mbed-src/common/board.o ./mbed-src/targets/cmsis/TARGET_NXP/TARGET_LPC176X/system_LPC17xx.o ./mbed-src/targets/cmsis/TARGET_NXP/TARGET_LPC176X/cmsis_nvic.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/i2c_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/analogin_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/gpio_irq_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/serial_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/sleep.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/pwmout_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/port_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/gpio_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/analogout_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/us_ticker.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/spi_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/rtc_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/ethernet_api.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/pinmap.o ./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/can_api.o ./NTPClient.o ./ComHandler.o ./tcpDataClient.o ./udpDataClient.o ./IMUSensor.o ./main.o ./HuaweiE372DongleInitializer.o ./SaraU2DongleInitializer.o ./GPSSensor.o ./GPSSensorUBX.o ./GPSSensorNMEA.o ./LSM303DLH.o ./L3GD20H.o ./Socket/Endpoint.o ./Socket/TCPSocketServer.o ./Socket/UDPSocket.o ./Socket/Socket.o ./Socket/TCPSocketConnection.o ./Pyrn3GModem/ATCommandsInterface.o ./Pyrn3GModem/PPPIPInterface.o ./Pyrn3GModem/PyrnUSBModem.o ./Pyrn3GModem/uBloxUSBModem.o ./Pyrn3GModem/USBSerialStream.o ./MODSERIAL/INIT.o ./MODSERIAL/MODSERIAL.o ./MODSERIAL/example3b.o ./MODSERIAL/RESIZE.o ./MODSERIAL/FLUSH.o ./MODSERIAL/PUTC.o ./MODSERIAL/ISR_TX.o ./MODSERIAL/example_dma.o ./MODSERIAL/ISR_RX.o ./MODSERIAL/GETC.o ./MODSERIAL/example1.o ./MODSERIAL/example2.o ./MODSERIAL/example3a.o ./MODSERIAL/MODSERIAL_IRQ_INFO.o ./3G/WanModem.o ./TinyGPS/TinyGPS.o ./HTTPClient/HTTPClient.o ./HTTPClient/data/HTTPMap.o ./HTTPClient/data/HTTPText.o ./HTTPClient/data/HTTPRawData.o ./mbed-rtos/rtos/Thread.o ./mbed-rtos/rtos/Semaphore.o ./mbed-rtos/rtos/Mutex.o ./mbed-rtos/rtos/RtosTimer.o ./MyThings/MySensor.o ./MyThings/MyDebug.o ./MyThings/MyBlinker.o ./MyThings/MyLibc.o ./MyThings/MyOsHelpers.o ./MyThings/MyThread.o ./MyThings/MyWatchdog.o ./MyThings/MyMemoryAllocator.o ./USBHost/USBHostSerial/USBHostSerial.o ./USBHost/FATFileSystem/FATDirHandle.o ./USBHost/FATFileSystem/FATFileSystem.o ./USBHost/FATFileSystem/FATFileHandle.o ./USBHost/FATFileSystem/ChaN/diskio.o ./USBHost/FATFileSystem/ChaN/ff.o ./USBHost/USBHostHub/USBHostHub.o ./USBHost/USBHostHID/USBHostKeyboard.o ./USBHost/USBHostHID/USBHostMouse.o ./USBHost/USBHostMIDI/USBHostMIDI.o ./USBHost/USBHost3GModule/WANDongle.o ./USBHost/USBHost3GModule/WANDongleSerialPort.o ./USBHost/USBHost/USBDeviceConnected.o ./USBHost/USBHost/USBHost.o ./USBHost/USBHost/USBHALHost_LPC17.o ./USBHost/USBHost/USBHALHost_RZ_A1.o ./USBHost/USBHost/USBEndpoint.o ./USBHost/USBHostMSD/USBHostMSD.o ./mbed-src/common/Timeout.o ./mbed-src/common/InterruptIn.o ./mbed-src/common/Ticker.o ./mbed-src/common/BusOut.o ./mbed-src/common/CAN.o ./mbed-src/common/FileLike.o ./mbed-src/common/I2CSlave.o ./mbed-src/common/InterruptManager.o ./mbed-src/common/FileSystemLike.o ./mbed-src/common/SPI.o ./mbed-src/common/FunctionPointer.o ./mbed-src/common/LocalFileSystem.o ./mbed-src/common/Stream.o ./mbed-src/common/TimerEvent.o ./mbed-src/common/Timer.o ./mbed-src/common/SPISlave.o ./mbed-src/common/BusInOut.o ./mbed-src/common/FilePath.o ./mbed-src/common/SerialBase.o ./mbed-src/common/FileBase.o ./mbed-src/common/Ethernet.o ./mbed-src/common/I2C.o ./mbed-src/common/CallChain.o ./mbed-src/common/Serial.o ./mbed-src/common/RawSerial.o ./mbed-src/common/retarget.o ./mbed-src/common/BusIn.o ./CAN/CANCommon.o ./CAN/CANCorrelator.o ./CAN/CANVariationDetector.o ./CAN/CANVariableData.o ./CAN/CANSniffer.o ./CAN/CANInterface.o ./CAN/CANFifoMessage.o ./CAN/CANDiagCalculator.o ./CAN/CANCommunicators.o ./CAN/CANCommunicator6A.o ./CAN/CANCommunicator68.o ./CAN/CANDiagSensor.o ./CAN/CANRecorderCalculator.o ./CAN/CANRecorder.o storageBase.o sd.o
#./USBHost/FATFileSystem/ChaN/ccsbcs.o 
SYS_OBJECTS = 
INCLUDE_PATHS = -I. -I./Socket -I./Pyrn3GModem -I./Pyrn3GModem/core -I./lwip -I./lwip/core -I./lwip/core/ipv4 -I./lwip/core/snmp -I./lwip/api -I./lwip/netif -I./lwip/netif/ppp -I./lwip/include -I./lwip/include/ipv4 -I./lwip/include/ipv4/lwip -I./lwip/include/lwip -I./lwip/include/netif -I./lwip-sys -I./lwip-sys/arch -I./MODSERIAL -I./3G -I./TinyGPS -I./HTTPClient -I./HTTPClient/data -I./mbed-rtos -I./mbed-rtos/rtos -I./mbed-rtos/rtx -I./mbed-rtos/rtx/TARGET_CORTEX_M -I./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3 -I./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3/TOOLCHAIN_GCC -I./MyThings -I./USBHost -I./USBHost/USBHostSerial -I./USBHost/FATFileSystem -I./USBHost/FATFileSystem/ChaN -I./USBHost/USBHostHub -I./USBHost/USBHostHID -I./USBHost/USBHostMIDI -I./USBHost/USBHost3GModule -I./USBHost/USBHost -I./USBHost/USBHostMSD -I./mbed-src -I./mbed-src/common -I./mbed-src/hal -I./mbed-src/api -I./mbed-src/targets -I./mbed-src/targets/cmsis -I./mbed-src/targets/cmsis/TARGET_NXP -I./mbed-src/targets/cmsis/TARGET_NXP/TARGET_LPC176X -I./mbed-src/targets/cmsis/TARGET_NXP/TARGET_LPC176X/TOOLCHAIN_GCC_ARM -I./mbed-src/targets/hal -I./mbed-src/targets/hal/TARGET_NXP -I./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X -I./mbed-src/targets/hal/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768 -I./CAN -I./FATFileSystem
LIBRARY_PATHS = 
LIBRARIES = 
LINKER_SCRIPT = ./mbed-src/targets/cmsis/TARGET_NXP/TARGET_LPC176X/TOOLCHAIN_GCC_ARM/LPC1768.ld

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE 	= $(GCC_BIN)arm-none-eabi-size

CPU = -mcpu=cortex-m3 -mthumb
CC_FLAGS = $(CPU) -c -g -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer
CC_FLAGS += -MMD -MP
CC_SYMBOLS = -DTARGET_LPC1768 -DTARGET_M3 -DTARGET_CORTEX_M -DTARGET_NXP -DTARGET_LPC176X -DTARGET_MBED_LPC1768 -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -D__CORTEX_M3 -DARM_MATH_CM3 -DMBED_BUILD_TIMESTAMP=1429019931.61 -D__MBED__=1 

LD_FLAGS = $(CPU) -Wl,--gc-sections --specs=nano.specs -u _printf_float -u _scanf_float -Wl,--wrap,main
LD_FLAGS += -Wl,-Map=$(PROJECT).map,--cref
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

ifeq ($(DEBUG), 1)
  CC_FLAGS += -DDEBUG -O0
else
  CC_FLAGS += -DNDEBUG -Os
endif

all: $(PROJECT).bin $(PROJECT).hex 

clean:
	rm -f $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(DEPS)
	rm *.lib

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 -fno-rtti $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(LIBRARIES) $(LD_SYS_LIBS)
	@echo ""
	@echo "*****"
	@echo "***** You must modify vector checksum value in *.bin and *.hex files."
	@echo "*****"
	@echo ""
	$(SIZE) -Ax $@

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size:
	$(SIZE) $(PROJECT).elf

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
