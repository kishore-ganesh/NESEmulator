#ifndef PPU_INCLUDE
#define PPU_INCLUDE
#include "memory.h"
class Memory; // why did forward declaration work
class PPU{
    char vram[2*1024];
    char registers[8];
    char OAM[256];
    Memory* memory;
    public:
    PPU(Memory* memory);
    char readAddress(unsigned short address);
    void cycle();
};

/*
PPU has both RAM 

 */

#endif