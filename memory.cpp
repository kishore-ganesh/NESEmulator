#include "memory.h"
using std::cout;
using std::endl;

Memory::Memory(char* path){
    cartridge = new Cartridge(path);
    this->ppu = ppu;
}

void Memory::setPPU(PPU* ppu){
    this->ppu = ppu;
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
        cout << "PPU access" <<endl;
    }
    // have mirroring

    else if(address==0x4016|| address == 0x4017){
        cout << "INPUT" << endl;
    }
    else if(address>=0x8000&&address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        return cartridge->readPRGAddress(prgRomAddress);
    }
}


unsigned char Memory::readCHRAddress(unsigned short address){
    return cartridge->readCHRAddress(address);
}

void Memory::writeAddress(unsigned short address, char value){
    printf("Writing to: %x\n",address);
    
    if(address <= 0x1FFF){
        memory[address%0x800] = value;
    }
    //add for PPU registers
    
    else if(address >= 0x2000 && address <= 0x2007){
        ppu->writeRegister((Registers)(address-0x2000), value);
    }

    // Have mirroring here
    else if(address == 0x4014){
        OAMDMA(value);
    }
    else if(address>=0x8000&& address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        // cartridge->write(prgRomAddress, value);
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
        printf("%d\n", i);
        char data = readAddress((highByte<<8) | i); //Check that OAM DMA increases OAM Addresses
        writeAddress(0x2004, data);
    }
}