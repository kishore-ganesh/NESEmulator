#ifndef MEMORY_INCLUDE
#define MEMORY_INCLUDE
#include<iostream>
#include "cartridge.h"
#include "ppu.h"
#include "controller.h"
#include "apu.h"
class PPU;
/*
Class for common memory bus operations ( this is shared between CPU, PPU and APU)
 */
class Memory{
    char memory[2*1024];
    PPU* ppu;
    Cartridge* cartridge;
    Controller* controller;
    APU* apu;
    public:
    Memory(char* path, Controller* controller, APU* apu);
    void setPPU(PPU* ppu);
    unsigned char readAddress(unsigned short address);
    unsigned char readCHRAddress(unsigned short address);
    void writeCHRAddress(unsigned short addres, char value);
    void writeAddress(unsigned short address, char value);
    short readLittleEndian(unsigned short address);
    void OAMDMA(unsigned char data);
};

#endif
