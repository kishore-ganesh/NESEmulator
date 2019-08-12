#include "memory.h"
using std::cout;
using std::endl;

Memory::Memory(char* path){
    cartridge = new Cartridge(path);
}
unsigned char Memory::readAddress(unsigned short address){
    if(address<=0x1FFF){
        return memory[address%(0x0800)];
    }
    else if(address==0x2002){
        return 0x80;
    }

    else if(address>=0x2000&&address<=0x3FFF){
        cout << "PPU access" <<endl;
    }

    else if(address==0x4016|| address == 0x4017){
        cout << "INPUT" << endl;
    }
    else if(address>=0x8000&&address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        return cartridge->read(prgRomAddress);
    }
}

void Memory::writeAddress(unsigned short address, char value){
    if(address <= 0x1FFF){
        memory[address%0x800] = value;
    }
    //add for PPU registers
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

void Memory::OAMDMA(char highByte){
    for(char i=0x00; i<=0xFF; i++){
        char data = readAddress((highByte<<8) | i); //Check that OAM DMA increases OAM Addresses
        writeAddress(0x2004, data);
    }
}