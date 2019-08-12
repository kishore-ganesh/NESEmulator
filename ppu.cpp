#include "ppu.h"

PPU::PPU(Cartridge* cartridge){
    this->cartridge = cartridge;
}

char PPU::readAddress(unsigned short address)
{
    if (address >= 0x2000 && address <= 0x2EFF){
        return vram[address - 0x2000];
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        return vram[address-0x3000];
    }
}