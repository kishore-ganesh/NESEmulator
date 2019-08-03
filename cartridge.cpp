#include <stdio.h>
#include "cartridge.h"

Cartridge::Cartridge(char *path)
{
    iNES_Header header;
    FILE *rom = fopen(path, "r");
    fread(&header, sizeof(header), 1, rom);
    if (header.flag[0] & 0x0004)
    {
        fseek(rom, 512, SEEK_CUR); //check this
    }

    long pos = ftell(rom);
    fread(PRG_ROM, header.prgSize * 0x4000, 1, rom);
    if (header.prgSize == 1)
    {
        fseek(rom, pos, SEEK_SET);
        fread(&PRG_ROM[0x4000], 0x4000, 1, rom);
    }
}

char Cartridge::read(short address)
{
    return PRG_ROM[address];
}

void Cartridge::write(short address, char value){
    PRG_ROM[address] = value;
}