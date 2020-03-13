#include "unrom_mapper.h"

unsigned char UNROM::readAddress(unsigned short address){
    // spdlog::info("UNROM READ ADDRESS: {0:x}", address + 0x8000);
    if(address + 0x8000 > 0xBFFF){
        address-= (0xC000 - 0x8000);
        return PRG_ROM_Bank[permanentBank][address];
    }
    else{
        return PRG_ROM_Bank[currentBank][address];
    }
}


void UNROM::writeAddress(unsigned short address, char value){
    char valueAtAddress = readAddress(address);
    if(valueAtAddress == value){
        currentBank = value & 0x07;
    }
    else{
        if(address + 0x8000 > 0xBFFF){
            address-= (0xC000 - 0x8000);
            PRG_ROM_Bank[permanentBank][address] = value;
        }
        else{
            PRG_ROM_Bank[currentBank][address] = value;
        }
    }

}

unsigned char UNROM::readCHRAddress(unsigned short address){
    return CHR_RAM[address];
}

void UNROM::writeCHRAddress(unsigned short address, char value){
    CHR_RAM[address] = value;
}