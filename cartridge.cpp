#include <stdio.h>
#include "cartridge.h"

Cartridge::Cartridge(char *path)
{
    iNES_Header header;
    FILE *rom = fopen(path, "r");
    fread(&header, sizeof(header), 1, rom);
    printHeader(header);
    if (header.flag[0] & 0x0004)
    {
        fseek(rom, 512, SEEK_CUR); //check this
    }

    long pos = ftell(rom);
    fread(PRG_ROM, header.prgSize * 0x4000, 1, rom);
    if (header.prgSize == 1)
    {
        fseek(rom, pos, SEEK_SET);
        fread(&PRG_ROM[0x4000], 0x4000, 1, rom); //check this
    }

    fread(CHR_ROM, header.chrSize*8192, 1, rom);
}

void Cartridge::printHeader(iNES_Header header){
    printf(
        "Cartridge program size is: %d\n\
        CHR ROM Size is: %d\n\
        Mirroring (0: Horizontal, 1: Vertical): %d\n\
        PRG RAM present: %d\n\
        Trainer present: %d\n\
        Ignore mirroring: %d\n\
        ",
        header.prgSize,
        header.chrSize,
        (header.flag[0] & 0x01),
        (header.flag[0] & 0x02) >> 1,
        (header.flag[0] & 0x04) >> 2,
        (header.flag[0] & 0x08) >> 3
    );
}

unsigned char Cartridge::readPRGAddress(unsigned short address)
{
    return PRG_ROM[address];
}

unsigned char Cartridge::readCHRAddress(unsigned short address){
    /* Have to add */
    return CHR_ROM[address];
}

void Cartridge::write(unsigned short address, char value){
    PRG_ROM[address] = value;
}