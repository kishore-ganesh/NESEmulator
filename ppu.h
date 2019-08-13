#ifndef PPU_INCLUDE
#define PPU_INCLUDE
#include "memory.h"
class Memory; // why did forward declaration work
enum Registers{
    PPUCTRL,
    PPUMASK,
    PPUSTATUS,
    OAMADDR,
    OAMDATA,
    PPUSCROLL,
    PPUADDR,
    PPUDATA,
};
class PPU{
    char vram[2*1024];
    char registers[8];
    short scroll, address;
    char xscroll, yscroll;
    char OAM[256];
    Memory* memory;
    public:
    PPU(Memory* memory);
    char readAddress(unsigned short address);
    void writeAddress(unsigned short address, char value);
    char getRegister(Registers reg);
    void setRegister(Registers reg, char value);
    char getIncrement();
    /*
    Read register is used when CPU reads something from PPU. It is
    an operation, unlike get register, which just gets the register without doing anything PPU specific
     */
    char readRegister(Registers reg);
    void writeRegister(Registers reg, char value);
    void cycle();
};

/*
PPU has both RAM 

 */

#endif