#ifndef MEMORY_INCLUDE
#define MEMORY_INCLUDE
#include<iostream>
#include "cartridge.h"
#include "ppu.h"
class PPU;
/*
Class for common memory bus operations ( this is shared between CPU, PPU and APU)
 */
class Memory{
    char memory[2*1024];
    PPU* ppu;
    Cartridge* cartridge;
    public:
    Memory(char* path);
    void setPPU(PPU* ppu);
    unsigned char readAddress(unsigned short address);
    unsigned char readCHRAddress(unsigned short address);
    void writeAddress(unsigned short address, char value);
    short readLittleEndian(unsigned short address);
    void OAMDMA(unsigned char data);
};

#endif
