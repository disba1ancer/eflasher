#ifndef ECFUNC_H
#define ECFUNC_H

#include <stdint.h>

enum ec_Addrs {
    PMU = 0xFF0C,
    WDTCFG = 0xFE80,
    FAN_CTRL = 0xFF11,
    MAPCFG = 0xFEA0,
    SPICFG = 0xFEAD,
    SPIADDR0 = 0xFEA8,
    SPIADDR1 = 0xFEA9,
    SPIADDR2 = 0xFEAA,
    SPICMD = 0xFEAC,
    SPIDATA = 0xFEAB,
};

enum ec_SPIcfgBits {
    BUSYCHK = (1U << 0),
    SPIBUSY = (1U << 1),
    DUMMYFLG = (1U << 2),
    SPICMDEN = (1U << 3),
    SPICSLOW = (1U << 4),
};

enum ec_SPICommands {
    WRITE = 2,
    READ = 3,
    WRDSBL = 4,
    RDSR = 5,
    WREN = 6,
    FREAD = 0xB,
    SECTOR_ERASE = 0x20,
};

struct ec_StateBackup {
    uint8_t PMUcfg;
    uint8_t WDTcfg;
    uint8_t SPIcfg;
};

extern uint16_t ADDRESS_PORT_HIGH, ADDRESS_PORT_LOW, DATA_PORT;

uint8_t                 ec_readByte         (uint16_t address);
void                    ec_writeByte        (uint16_t address, uint8_t byte_val);
void                    ec_selectAddress    (uint16_t address);
uint8_t                 ec_readByteNA       (void);
void                    ec_writeByteNA      (uint8_t byte_val);
struct ec_StateBackup*  ec_enterProgMode    (struct ec_StateBackup *state);
void                    ec_exitProgMode     (struct ec_StateBackup const *state);
void                    ec_enableSPI        (void);
void                    ec_disableSPI       (void);
void                    ec_enableDummyByte  (void);
void                    ec_disableDummyByte (void);
void                    ec_selectSPIAddress (uint32_t address);
void                    ec_sendSPICommand   (uint8_t command);
void                    ec_waitForSPI       (void);
void                    ec_forceSPICSLow    (void);
void                    ec_forceSPICSHigh   (void);

#endif

