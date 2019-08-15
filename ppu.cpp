#include "ppu.h"
PPU::PPU(Memory* memory, EdgeInterrupt* NMI){
    this->memory = memory;
    this->NMI = NMI;
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("NESEmulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 240, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
}

char PPU::readAddress(unsigned short address)
{
    if(address>=0x0000&&address<=0x1FFF){
        return memory->readCHRAddress(address);
    }
    if (address >= 0x2000 && address <= 0x2EFF){
        return vram[address - 0x2000];
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        return vram[address-0x1000];
    }
    if (address>=0x3F00&&address<=0x3F1F){
        /*  retyurn pallete */
    }

    if(address >= 0x3520 && address <= 0x3FFF){
        /* Return pallete -  */
    }
}

void PPU::writeAddress(unsigned short address, char value){
    if (address >= 0x2000 && address <= 0x2EFF){
        vram[address - 0x2000] = value;
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        vram[address-0x1000] = value;
    }

    if (address>=0x3F00&&address<=0x3F1F){
        /*  retyurn pallete */
    }

    if(address >= 0x3520 && address <= 0x3FFF){
        /* Return pallete -  */
    }
}


char PPU::getRegister(Registers reg){
    return registers[reg];
}

void PPU::setRegister(Registers reg, char value){
    registers[reg] = value;
}

char PPU::getIncrement(){
    if(PPUCTRL&0x02){
        return 32;
    }
    return 1;
}

char PPU::readRegister(Registers reg){
    char value = getRegister(reg);
    switch(reg){
        case PPUSTATUS: {
            setRegister(reg, value & 0x7F);
            break; 
        }
        case OAMDATA: {
            return OAM[value]; 
        }
        case PPUDATA: {
            char increment = getIncrement();
            char address = getRegister(PPUADDR);
            setRegister(PPUADDR,address+increment);
            break;
        }
    }

    return value;
}

void PPU::writeRegister(Registers reg, char value){
    setRegister(reg, value);
    switch(reg){
        case OAMDATA:{
            char address = getRegister(OAMADDR);
            setRegister(PPUADDR, address + 1);
        }
        case PPUSCROLL: {
            char existingScroll = (scroll&0xFF00) >> 8;
            scroll = existingScroll | (value << 8);
            xscroll = existingScroll;
            yscroll = value; 
        }
        case PPUADDR: {
            address = (address << 8) | value;
            address = (address%0x4000);
        }
        case PPUDATA: {
            writeAddress(address, value);
            char increment = getIncrement();
            setRegister(PPUADDR, address + increment);
        }
    }
}

short PPU::getNameTableAddress(){
        char nameTableNumber = getRegister(PPUCTRL) & 0x03;
        short baseAddress = 0x2000;
        short address = baseAddress + ((nameTableNumber*4) << 8);
        return 0x2000 + address; //check
}

short PPU::getBasePatternTableAddress(bool background){
    char mask = 0x10;
    if(!background){
        mask = 0x01;
    }
    if(PPUCTRL&mask){
        return 0x0000;
    }
    else{
        return 0x1000;
    }   
}

void PPU::generateFrame(){
    short baseNameTableAddress = getNameTableAddress();
    //check for enable rednering
    for(short i = baseNameTableAddress; i<=baseNameTableAddress+960; i++){
        char nameTableEntry = readAddress(i);
        short basePatternTableAddress = getBasePatternTableAddress(true);
        short baseAttributeTableAddress = i + 0x3C0; //check this, make this only one memory acces
        short attributeTableAddressOffset = (((i-baseNameTableAddress)%32)/2)+(i-baseNameTableAddress)/4;
        char attributeEntry = readAddress(baseAttributeTableAddress + attributeTableAddressOffset);
        char offset = ((i-baseNameTableAddress)%30) - (attributeTableAddressOffset - (i-baseNameTableAddress)/4)*2;
        char attribute = (attributeEntry & (0x03 << offset*2)) >> (offset*2);
        for(char j = 0; j < 8; j++){
            short patternAddress = nameTableEntry*16 + basePatternTableAddress + j;
            char upperTile = readAddress(patternAddress);
            char lowerTile = readAddress(patternAddress+8);
            
            for(char k = 0; k < 8; k++){
                short palleteAddress = 0x3F00 + (attribute << 2) | ((upperTile & 0x01) << 1 )| (lowerTile & 0x01);    
                char palleteIndex = readAddress(palleteAddress);
                int x = k + (i%32)*8;
                
                int y = (i/32)*8+j;
                setPixel(x, y, palletes[palleteIndex]); // need to refactor
            }
        }
    }
    SDL_Delay(500); // temp
    NMI->triggerInterrupt();
}

RGB PPU::getPixel(int x, int y){
    return display[x][y];
}

void PPU::setPixel(int x, int y, RGB value){
    display[x][y] = value;
}
void PPU::displayFrame(){
    for(int x = 0; x < 256; x++){
        for(int y = 0; y < 240; y++){
            RGB pixel = getPixel(x, y);
            SDL_SetRenderDrawColor(renderer, pixel.r, pixel.g, pixel.b, 1);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}
/*
PPU CHR ROM should be mapped to the pattern tables
We also need to check if it is CHR RAM instead

PPU Scroll we need to implement such that it takes place next frame
 */

/*
Check if PPU mapping is 
Implement mirroring

If NMI it is set, only then should you trigger NMI
 */