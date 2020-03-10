#include "memory.h"
using std::cout;
using std::endl;

Memory::Memory(char* path, Controller* controller){
    cartridge = new Cartridge(path);
    memset(memory, 2*1024, 0);
    // this->ppu = ppu;
    this->controller = controller;
}

void Memory::setPPU(PPU* ppu){
    this->ppu = ppu;
    ppu->setMirroringMode(cartridge->getMirroringMode());
}
unsigned char Memory::readAddress(unsigned short address){
    if(address<=0x1FFF){
        return memory[address%(0x0800)];
    }
    // else if(address==0x2002){
    //     return 0x80;
    // }

    else if(address>=0x2000&&address<=0x2007){
        return ppu->readRegister((Registers)(address - 0x2000));
        SPDLOG_INFO("PPU access");
    }
    else if(address >= 0x2008 && address <= 0x3FFF){
        return ppu->readRegister((Registers)(address % 8));
    }
    // have mirroring

    else if(address==0x4016){
        SPDLOG_INFO("Input address: {0:x}", address);
        SPDLOG_INFO("INPUT");
        return controller->readNext();
    }
    else if(address == 0x4017){
        return 0;
    }
    else if(address<=0x401f){

    }
    else if(address>=0x8000&&address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        return cartridge->readPRGAddress(prgRomAddress);
    }
    else{
        spdlog::error("Error, Memory Address out of range");
    }
}


unsigned char Memory::readCHRAddress(unsigned short address){
    return cartridge->readCHRAddress(address);
}

void Memory::writeAddress(unsigned short address, char value){
    SPDLOG_INFO("Writing to: {0:x} value: {1:x}",address, value);
    if(address == 0x2000){
        SPGLOG_INFO("Writing to PPUCTRL: {0:x}\n", value);
    }
    if(address >= 0x2008 && address <= 0x3fff){
        SPDLOG_INFO("Mirrored register access");
    }    
    else if(address <= 0x1FFF){
        memory[address%0x800] = value;
    }
    //add for PPU registers
    
    else if(address >= 0x2000 && address <= 0x2007){
        ppu->writeRegister((Registers)(address-0x2000), value);
    }

    else if(address >= 0x2008 && address <= 0x3FFF){
        ppu->writeRegister((Registers)(address % 8), value);
    }


    // Have mirroring here  
    else if(address == 0x4014){
        OAMDMA(value);
    }

    // else if(address >= 0x4015 && address)
    else if(address>=0x8000&& address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        // cartridge->write(prgRomAddress, value);
    }
    else{
        // spdlog::error("Invalid Write Address: {0:x}", address);
    }

}

short Memory::readLittleEndian(unsigned short address){
    short data = 0;
    data = (unsigned char)readAddress(address+1);
    data = (data)<<8;
    data = data | (unsigned char)readAddress(address); // sign bit extended here
    return data;
}

void Memory::OAMDMA(unsigned char highByte){
    for(unsigned short i=0x00; i<=(unsigned short)0xFF; i++){
        // printf("OAM WRITING FROM: %x\n", (highByte<<8)|i);
        char data = readAddress((highByte<<8) | i); //Check that OAM DMA increases OAM Addresses
        writeAddress(0x2004, data);
    }
}