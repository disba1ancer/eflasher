#include "ecfunc.h"

uint16_t    ADDRESS_PORT_HIGH = 0xFF29,
            ADDRESS_PORT_LOW = 0xFF2A,
            DATA_PORT = 0xFF2B;

uint8_t ec_readByte(uint16_t address) {
    uint8_t result;
    __asm {
        mov ax, address
        xchg al, ah
        mov dx, ADDRESS_PORT_HIGH
        out dx, ax
        mov dx, DATA_PORT
        in al, dx
        mov result, al
    };
    return result;
}

void ec_writeByte(uint16_t address, uint8_t byte_val) {
    __asm {
        mov ax, address
        xchg al, ah
        mov dx, ADDRESS_PORT_HIGH
        out dx, ax
        mov dx, DATA_PORT
        mov al, byte_val
        out dx, al
    };
}

void ec_selectAddress(uint16_t address) {
    __asm {
        mov ax, address
        xchg al, ah
        mov dx, ADDRESS_PORT_HIGH
        out dx, ax
    }
}

uint8_t ec_readByteNA(void) {
    uint8_t result;
    __asm {
        mov dx, DATA_PORT
        in al, dx
        mov result, al
    }
    return result;
}

void ec_writeByteNA(uint8_t byte_val) {
    __asm {
        mov dx, DATA_PORT
        mov al, byte_val
        out dx, al
    }
}

struct ec_StateBackup* ec_enterProgMode (struct ec_StateBackup *state) {
    ec_selectAddress(PMU);
    state->PMUcfg = ec_readByteNA();
    ec_writeByteNA(0x40);
    
    ec_selectAddress(WDTCFG);
    state->WDTcfg = ec_readByteNA();
    ec_writeByteNA(0xC8);
    
    ec_selectAddress(SPICFG);
    state->SPIcfg = ec_readByteNA();
    
    ec_writeByte(FAN_CTRL, 0xFF);
    return state;
}

void ec_exitProgMode (struct ec_StateBackup const *state) {
    ec_writeByte(SPICFG, state->SPIcfg);
    ec_writeByte(PMU, state->PMUcfg);
    ec_writeByte(WDTCFG, state->WDTcfg);
}

void ec_enableSPI(void) {
    ec_selectAddress(SPICFG);
    ec_writeByteNA(ec_readByteNA() | (BUSYCHK | SPICMDEN));
}

void ec_disableSPI(void) {
    ec_selectAddress(SPICFG);
    ec_writeByteNA(ec_readByteNA() & ~(BUSYCHK | SPICMDEN));
}

void ec_enableDummyByte(void) {
    ec_selectAddress(SPICFG);
    ec_writeByteNA(ec_readByteNA() | DUMMYFLG);
}

void ec_disableDummyByte(void) {
}

void ec_selectSPIAddress(uint32_t address) {
    ec_writeByte(SPIADDR0, address);
    ec_writeByte(SPIADDR1, address >> 8);
    ec_writeByte(SPIADDR2, address >> 16);
}

void ec_sendSPICommand(uint8_t command) {
    ec_writeByte(SPICMD, command);
}

void ec_waitForSPI(void) {
    ec_selectAddress(SPICFG);
    while (ec_readByteNA() & SPIBUSY) {}
}

void ec_forceSPICSLow(void) {
    ec_selectAddress(SPICFG);
    ec_writeByteNA(ec_readByteNA() | SPICSLOW);
}

void ec_forceSPICSHigh(void) {
    ec_selectAddress(SPICFG);
    ec_writeByteNA(ec_readByteNA() & ~SPICSLOW);
}

