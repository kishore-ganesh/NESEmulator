#ifndef UNROM_INCLUDE
#define UNROM_INCLUDE
#include<stdio.h>
#include<stdlib.h>
#include "util.h"
class UNROM{
    unsigned char PRG_ROM_Bank[8][16*1024];
    unsigned char currentBank;
    unsigned char permanentBank;
    unsigned char CHR_RAM[8 * 1024];
    public:
    UNROM(FILE* rom){

        //Make this dynamic later
        fread(PRG_ROM_Bank,  8 * 0x4000, 1, rom);
        currentBank = 0;
        permanentBank = 7;
        
    }
    unsigned char readAddress(unsigned short address);

    void writeAddress(unsigned short address, char value);
    unsigned char readCHRAddress(unsigned short address);
    void writeCHRAddress(unsigned short address, char value);
};


#endif
