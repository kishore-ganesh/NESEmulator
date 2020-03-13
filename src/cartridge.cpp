#include <stdio.h>
#include<algorithm>
#include "cartridge.h"


Cartridge::Cartridge(char *path)
{
    FILE *rom = fopen(path, "r");
    fread(&header, sizeof(header), 1, rom);
    type = (Mapper) (header.flag[0] >> 4);
    spdlog::info("Cartridge type: {0:d}", type);
    switch(type){
        case Mapper::NROM: {
            PRG_ROM = new char[std::max(header.prgSize * 0x4000, 32768)];
            printHeader(header);
            if (header.flag[0] & 0x0004)
            {
                fseek(rom, 512, SEEK_CUR); //check this
            }

            long pos = ftell(rom);
            fread(PRG_ROM, header.prgSize * 0x4000, 1, rom);
            //This done due to mirroring
            if (header.prgSize == 1)
            {
                fseek(rom, pos, SEEK_SET);
                fread(&PRG_ROM[0x4000], 0x4000, 1, rom); //check this
            }

            fread(CHR_ROM, header.chrSize*8192, 1, rom);
                break;
        }
    
    case Mapper::UNROM: {
            unrom = new UNROM(rom);
            break;
        }
    }

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
    switch(type){
        case Mapper::NROM: {return PRG_ROM[address]; break;}
        case Mapper::UNROM: {return unrom->readAddress(address); break;}
    }
    
}

unsigned char Cartridge::readCHRAddress(unsigned short address){
    /* Have to add */
    switch(type){
        case Mapper::NROM: {return CHR_ROM[address]; break;}
        case Mapper::UNROM: {return unrom->readCHRAddress(address); break;};
    }
    
}

void Cartridge::writeCHRAddress(unsigned short address, unsigned char value){
    switch(type){
        case Mapper::UNROM: {
            unrom->writeCHRAddress(address, value);
        }
    }
}

void Cartridge::write(unsigned short address, char value){
    switch(type){
        case Mapper::NROM: {break;}
        case Mapper::UNROM: {unrom->writeAddress(address, value); break;}
    }
    // PRG_ROM[address] = value;
}   

bool Cartridge::getMirroringMode(){
    return (header.flag[0] & 0x01);
}