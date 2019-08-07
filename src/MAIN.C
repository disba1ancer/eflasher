#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecfunc.h"

enum ReturnErrors {
    COMMAND_NOT_GIVEN = 1,
    FILE_NOT_GIVEN,
    OPEN_FILE_FAIL,
};

enum Constants {
    SECTOR_SIZE = 0x1000,
    REGION_SIZE = 0x4000,
    REGIONS_COUNT = 8,
};

uint8_t readStatusRegister(void);
void readSector(uint8_t (*buffer)[SECTOR_SIZE]);
void writeSector(const uint8_t (*buffer)[SECTOR_SIZE]);
//int appendByte(uint8_t* buf, size_t bufSize, size_t* curPos, uint8_t value);

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "rdsr")) {
            printf("%X\n", (unsigned)readStatusRegister());
        } else if (!strcmp(argv[1], "read")) {
            if (argc >= 3) {
                FILE* file;
                if (file = fopen(argv[2], "wb")) {
                    uint8_t (*buffer)[SECTOR_SIZE] = (uint8_t (*)[SECTOR_SIZE])malloc(sizeof(uint8_t[SECTOR_SIZE]));
                    readSector(buffer);
                    fwrite(*buffer, sizeof(uint8_t), SECTOR_SIZE, file);
                    free(buffer);
                    fclose(file);
                } else {
                    printf("Could not open the file");
                    return OPEN_FILE_FAIL;
                }
            } else {
                printf("File name not given");
                return FILE_NOT_GIVEN;
            }
        } else if (!strcmp(argv[1], "write")) {
            if (argc >= 3) {
                FILE* file;
                if (file = fopen(argv[2], "rb")) {
                    uint8_t (*buffer)[SECTOR_SIZE] = (uint8_t (*)[SECTOR_SIZE])malloc(sizeof(uint8_t[SECTOR_SIZE]));
                    fread(*buffer, sizeof(uint8_t), SECTOR_SIZE, file);
                    writeSector(buffer);
                    free(buffer);
                    fclose(file);
                } else {
                    printf("Could not open the file");
                    return OPEN_FILE_FAIL;
                }
            } else {
                printf("File name not given");
                return FILE_NOT_GIVEN;
            }
        }
    } else {
        printf("Command not given");
        return COMMAND_NOT_GIVEN;
    }
    return 0;
}

uint8_t readStatusRegister(void) {
    struct ec_StateBackup oldState;
    uint8_t result;
    ec_enterProgMode(&oldState);
    ec_disableDummyByte();
    ec_enableSPI();
    ec_waitForSPI();
    ec_sendSPICommand(RDSR);
    ec_waitForSPI();
    ec_disableSPI();
    result = ec_readByte(SPIDATA);
    ec_exitProgMode(&oldState);
    return result;
}

void readSector(uint8_t (*buffer)[SECTOR_SIZE]) {
    struct ec_StateBackup oldState;
    int i;
    ec_enterProgMode(&oldState);
    ec_enableSPI();
    ec_enableDummyByte();
    ec_waitForSPI();
    for (i = 0; i < SECTOR_SIZE; ++i) {
        ec_waitForSPI();
        ec_selectSPIAddress(0x1F000 + (unsigned)i);
        ec_sendSPICommand(FREAD);
        ec_waitForSPI();
        (*buffer)[i] = ec_readByte(SPIDATA);
    }
    ec_disableSPI();
    ec_exitProgMode(&oldState);
}

void writeSector(const uint8_t (*buffer)[SECTOR_SIZE]) {
    struct ec_StateBackup oldState;
    int i;
    ec_enterProgMode(&oldState);
    ec_enableSPI();
    ec_disableDummyByte();
    ec_waitForSPI();
    ec_sendSPICommand(WREN);
    /*ec_waitForSPI();
    ec_selectSPIAddress(0x1F000);
    ec_sendSPICommand(SECTOR_ERASE);*/
    for (i = 0; i < SECTOR_SIZE; ++i) {
        /*do {
            ec_sendSPICommand(RDSR);
            ec_waitForSPI();
        } while (!(ec_readByte(SPIDATA) & 1));*/
        ec_waitForSPI();
        ec_sendSPICommand(WREN);
        ec_waitForSPI();
        ec_selectSPIAddress(0x1F000 + (unsigned)i);
        ec_writeByte(SPIDATA, (*buffer)[i]);
        ec_sendSPICommand(WRITE);
    }
    ec_waitForSPI();
    ec_sendSPICommand(WRDSBL);
    ec_disableSPI();
    ec_exitProgMode(&oldState);
}

/*int appendByte(uint8_t* buf, size_t bufSize, size_t* curPos, uint8_t value) {
    buf[*curPos] = value;
    ++*curPos;
    return *curPos >= bufSize;
}*/

