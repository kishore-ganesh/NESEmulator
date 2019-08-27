#include "ppu.h"

char getOffset(char r, char c){
    char sum = r + c;
    if(r==1){
        sum+=1;
    }
    return sum;
}
PPU::PPU(Memory* memory, EdgeInterrupt* NMI){
    this->memory = memory;
    this->NMI = NMI;
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("NESEmulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 240, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
}

unsigned char PPU::readAddress(unsigned short address)
{
    // std::cout << address << std::endl;
    address = (address % 0x4000);
    if(address>=0x0000&&address<=0x1FFF){
        return memory->readCHRAddress(address);
    }
    if (address >= 0x2000 && address <= 0x2FFF){
        return vram[address - 0x2000];
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        return vram[address-0x1000];
    }
    if (address>=0x3F00&&address<=0x3F1F){
        return programPalletes[address-0x3F00];
        /*  retyurn pallete */
    }

    if(address >= 0x3520 && address <= 0x3FFF){
        return  programPalletes[(address- 0x3520) % 0x20];
        // return programPalletes[address-0x3F00]; // fix this
        /* Return pallete -  */
    }
}

void PPU::writeAddress(unsigned short address, char value){
    address = (address%0x4000);
    if (address >= 0x2000 && address <= 0x2FFF){
        vram[address - 0x2000] = value;
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        vram[address-0x3000] = value;
    }

    if (address>=0x3F00&&address<=0x3F1F){
        programPalletes[address-0x3F00] = value;
        /*  retyurn pallete */
    }

    if(address >= 0x3520 && address <= 0x3FFF){
        programPalletes[(address-0x3F20)%0x20] = value;
        /* Return pallete -  */
    }
}


unsigned char PPU::getRegister(Registers reg){
    return registers[reg];
}

void PPU::setRegister(Registers reg, char value){
    registers[reg] = value;
}

char PPU::getIncrement(){
    if(PPUCTRL&0x04){
        return 32;
    }
    return 1;
}

unsigned char PPU::readRegister(Registers reg){
    unsigned char value = getRegister(reg);
    switch(reg){
        case PPUSTATUS: {
            setRegister(reg, value & 0x7F);
            break; 
        }
        case OAMDATA: {
            return OAM[value]; 
        }
        case PPUDATA: {
            printf("READING FROM PPU DATA\n");
            char increment = getIncrement();
            // unsigned char currentAddress = getRegister(PPUADDR);
            // setRegister(PPUADDR,currentAddress+increment);
            address+=increment;
            break;
        }
    }

    return value;
}

void PPU::writeRegister(Registers reg, unsigned char value){
    setRegister(reg, value);
    switch(reg){
        case OAMDATA:{
            char address = getRegister(OAMADDR);
            setRegister(PPUADDR, address + 1);
            break;
        }
        case PPUSCROLL: {
            char existingScroll = (scroll&0xFF00) >> 8;
            scroll = existingScroll | (value << 8);
            xscroll = existingScroll;
            yscroll = value; 
            break;
        }
        case PPUADDR: {
            address = (address << 8) | value;
            address = (address%0x4000);
            printf("PPU ADDRESS NOW: %x\n", address);
            break;
        }
        case PPUDATA: {
            printf("WRITING TO PPU DATA: %x\n", address);
            writeAddress(address, value);
            char increment = getIncrement();
            // setRegister(PPUADDR, address + increment);
            address+=increment;
            break;
        }
    }
}

short PPU::getNameTableAddress(){
        char nameTableNumber = getRegister(PPUCTRL) & 0x03;
        short baseAddress = 0x2000;
        short address = baseAddress + ((nameTableNumber*4) << 8);
        return address; //check
}

short PPU::getBasePatternTableAddress(bool background){
    char mask = 0x10;
    if(!background){
        mask = 0x08;
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
    for(short i = baseNameTableAddress; i<baseNameTableAddress+960; i++){
        unsigned char nameTableEntry = readAddress(i);
        short basePatternTableAddress = getBasePatternTableAddress(true);
        short baseAttributeTableAddress = baseAttributeTableAddress + 0x3C0; //check this, make this only one memory acces
        short attributeNameTableAddress = ((i - baseNameTableAddress) % 32)/2 + ((i-baseNameTableAddress)/64)*16;
        short attributeTableAddressOffset = (((attributeNameTableAddress)%16)/2)+(attributeNameTableAddress/32)*8; //Fix calculation
        unsigned char attributeEntry = readAddress(baseAttributeTableAddress + attributeTableAddressOffset);

        char r, c;
        r = ((attributeNameTableAddress/16)&0x01)?1:0;
        c = (attributeNameTableAddress&0x01)?1:0;
        unsigned char offset = getOffset(r, c);
        unsigned char attribute = (attributeEntry & (0x03 << offset*2)) >> (offset*2); // check if this is correct for 2 tiles
        for(char j = 0; j < 8; j++){
            short patternAddress = nameTableEntry*16 + basePatternTableAddress + j;
            unsigned char upperTile = readAddress(patternAddress);
            unsigned char lowerTile = readAddress(patternAddress+8);
            
            for(char k = 0; k < 8; k++){
                short palleteAddress = 0x3F00 | ((attribute << 2) | ((lowerTile >> 7) << 1 )| (upperTile >> 7));   
                if((((lowerTile >> 7) << 1 )| ((upperTile >> 7))==0)){
                    palleteAddress = 0x3F00; // check this
                }
                upperTile<<=1;

                lowerTile<<=1; 
                char palleteIndex = readAddress(palleteAddress);
                int x = k + ((i-baseNameTableAddress)%32)*8;
                
                int y = ((i-baseNameTableAddress)/32)*8+j;
                setPixel(x, y, palletes[palleteIndex&0x3F]); // need to refactor
            }
        }
    }
    unsigned char status = getRegister(PPUSTATUS);
    setRegister(PPUSTATUS, status|0x80);
    /*
        Add sprite code here
     */
    // SDL_Delay(50); // temp
    NMI->triggerInterrupt();
}

RGB PPU::getPixel(int x, int y){
    return display[x][y];
}

void PPU::setPixel(int x, int y, RGB value){
    // std::cout << x << " " << y << std::endl;
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
    SDL_RenderPresent(renderer);
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