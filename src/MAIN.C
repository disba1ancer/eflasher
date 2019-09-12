#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ecfunc.h"

enum ReturnErrors {
    INVALID_ARGUMENT = 1,
    FILE_NOT_GIVEN,
    OPEN_FILE_FAIL,
};

enum Constants {
    REGION_SIZE = 0x4000,
    REGIONS_COUNT = 8,
    FLASH_SIZE = 0x20000,
    BUFFER_SIZE = 0x1000,
};

uint32_t SECTOR_SIZE = 0x1000;

int readMain2(char* file, uint32_t address, uint32_t size);
int writeMain2(char* file, uint32_t address, uint32_t size);
void writeSector(const uint8_t *buffer, int bSize, uint32_t address);

int main(int argc, char* argv[]) {
    char **argv2 = argv + 1, *arg;
    while (arg = *(argv2++)) {
        if (arg[0] == '-') {
            if (arg[1] == 'p') {
                if (arg = *(argv2++)) {
                    uint16_t indexIOBase;
                    if (sscanf(arg, "%"SCNx16, &indexIOBase)) {
                        ADDRESS_PORT_HIGH = ++indexIOBase;
                        ADDRESS_PORT_LOW = ++indexIOBase;
                        DATA_PORT = ++indexIOBase;
                    } else {
                        printf("Invalid argument sequence");
                        return INVALID_ARGUMENT;
                    }
                } else {
                    printf("Invalid argument sequence");
                    return INVALID_ARGUMENT;
                }
            } else if (arg[1] == 's') {
                if (arg = *(argv2++)) {
                    unsigned pow;
                    if (sscanf(arg, "%u", &pow)) {
                        SECTOR_SIZE = 1 << pow;
                    } else {
                        printf("Invalid argument sequence");
                        return INVALID_ARGUMENT;
                    }
                } else {
                    printf("Invalid argument sequence");
                    return 1;
                }
            } else if (arg[1] == 'r') {
                if (arg = *(argv2++)) {
                    char* file = arg;
                    uint32_t address = 0;
                    uint32_t size = 0;
                    if (arg = *(argv2++)) {
                        if (!sscanf(arg, "%"SCNx32, &address)) {
                            printf("Invalid argument sequence");
                            return INVALID_ARGUMENT;
                        }
                    }
                    if (arg = *(argv2++)) {
                        if (sscanf(arg, "%"SCNx32, &size)) {
                            printf("Invalid argument sequence");
                            return INVALID_ARGUMENT;
                        }
                    }
                    return readMain2(file, address, size);
                } else {
                    printf("File name not given");
                    return FILE_NOT_GIVEN;
                }
            } else if (arg[1] == 'w') {
                if (arg = *(argv2++)) {
                    char* file = arg;
                    uint32_t address = 0;
                    uint32_t size = 0;
                    if (arg = *(argv2++)) {
                        if (!sscanf(arg, "%"SCNx32, &address)) {
                            printf("Invalid argument sequence");
                            return INVALID_ARGUMENT;
                        }
                    }
                    if (arg = *(argv2++)) {
                        if (!sscanf(arg, "%"SCNx32, &size)) {
                            printf("Invalid argument sequence");
                            return INVALID_ARGUMENT;
                        }
                    }
                    return writeMain2(file, address, size);
                } else {
                    printf("File name not given");
                    return FILE_NOT_GIVEN;
                }
            }
        } else {
            printf("Invalid argument sequence");
            return INVALID_ARGUMENT;
        }
    }
    return 0;
}

int readMain2(char* filename, uint32_t address, uint32_t size) {
    FILE* file;
    if (file = fopen(filename, "wb")) {
        uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t[BUFFER_SIZE]));
        size_t bufPos = 0;
        uint8_t command = FREAD;
        struct ec_StateBackup oldState;
        uint32_t i;
        if (size == 0) size = 0x20000;
        ec_enterProgMode(&oldState);
        ec_enableSPI();
        ec_enableDummyByte();
        for (i = 0; i < size; ++address) {
            ec_waitForSPI();
            ec_selectSPIAddress(i);
            ec_sendSPICommand(command);
            ec_waitForSPI();
            buffer[bufPos] = ec_readByte(SPIDATA);
            if (++bufPos >= BUFFER_SIZE || ++i >= size) {
                fwrite(buffer, sizeof(uint8_t), bufPos, file);
                fflush(file);
                bufPos = 0;
            }
        }
        ec_disableSPI();
        ec_exitProgMode(&oldState);
        free(buffer);
        fclose(file);
    } else {
        printf("Could not open the file");
        return OPEN_FILE_FAIL;
    }
    return 0;
}

int writeMain2(char* filename, uint32_t address, uint32_t size) {
    FILE* file;
    if (file = fopen(filename, "rb")) {
        uint8_t *buffer = (uint8_t*)malloc(SECTOR_SIZE);
        struct ec_StateBackup oldState;
        uint32_t i;
        ec_enterProgMode(&oldState);
        address &= ~(SECTOR_SIZE - 1);
        if (size == 0) size = 0x20000;
        ec_enableSPI();
        for (i = 0; i < size && !feof(file) && !ferror(file); i += SECTOR_SIZE, address += SECTOR_SIZE) { // TODO: rewrite write cycle
            uint32_t size = fread(buffer, sizeof(uint8_t), SECTOR_SIZE, file);
            writeSector(buffer, size, address);
        }
        ec_disableSPI();
        ec_exitProgMode(&oldState);
        free(buffer);
        fclose(file);
    } else {
        printf("Could not open the file");
        return OPEN_FILE_FAIL;
    }
    return 0;
}

void writeSector(const uint8_t *buffer, int bSize, uint32_t address) {
    int i;
    ec_disableDummyByte();
    ec_waitForSPI();
    ec_sendSPICommand(WREN);
    ec_waitForSPI();
    ec_selectSPIAddress(address);
    ec_sendSPICommand(SECTOR_ERASE);
    for (i = 0; i < bSize; ++i, ++buffer, ++address) {
        ec_waitForSPI();
        ec_sendSPICommand(WREN);
        ec_waitForSPI();
        ec_selectSPIAddress(address);
        ec_writeByte(SPIDATA, *buffer);
        ec_sendSPICommand(WRITE);
    }
    ec_waitForSPI();
    ec_sendSPICommand(WRDSBL);
}

