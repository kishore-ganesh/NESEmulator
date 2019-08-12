#include "cartridge.h"
class PPU{
    char vram[2*1024];
    char registers[8];
    char OAM[256];
    Cartridge* cartridge;
    PPU(Cartridge* cartridge);
    char readAddress(unsigned short address);
    void cycle();
};

/*
PPU has both RAM 

 */