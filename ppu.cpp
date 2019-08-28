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
    this->cyclesLeft = 0;
    this->currentCycle = 0;
    this->currentScanline = -1;
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

bool PPU::shouldInterrupt(){
    unsigned char value = getRegister(PPUCTRL);
    return value & 0x80;
}

void PPU::addCycles(int cycles){
    currentCycle+=cycles;
    cyclesLeft-=cycles;
}

void PPU::fetchTile(int tileNumber){
    // Fix this
    // 
    short baseNameTableAddress = getNameTableAddress();
    unsigned char nameTableEntry = readAddress(tileNumber+baseNameTableAddress+(currentScanline/8)*32);    
    short basePatternTableAddress = getBasePatternTableAddress(true);
    short baseAttributeTableAddress = baseNameTableAddress + 0x3C0; //check this, make this only one memory acces
    short attributeNameTableAddress = (((currentScanline/8)*32+tileNumber) % 32)/2 + (((currentScanline/8)*32+tileNumber)/64)*16;
    short attributeTableAddressOffset = (((attributeNameTableAddress)%16)/2)+(attributeNameTableAddress/32)*8; //Fix calculation
    unsigned char attributeEntry = readAddress(baseAttributeTableAddress + attributeTableAddressOffset);
    char r, c;
    r = ((attributeNameTableAddress/16)&0x01)?1:0;
    c = (attributeNameTableAddress&0x01)?1:0;
    unsigned char offset = getOffset(r, c);
    attribute = (attributeEntry & (0x03 << offset*2)) >> (offset*2); // check if this is correct for 2 tiles
    short patternAddress = nameTableEntry*16 + basePatternTableAddress + (currentScanline%8); // Should not be current scanline
    upperPattern|=(readAddress(patternAddress)) << 8;
    lowerPattern|=(readAddress(patternAddress+8)) << 8;
}

void PPU::generateFrame(int cycles){
    cyclesLeft += cycles;
    printf("CURRENT CYCLE: %d, CYCLES LEFT: %d\n", currentCycle, cyclesLeft);
    if(currentCycle==0){
        if(cyclesLeft>=1){
            addCycles(1);
            renderFlag = false;
        }
        else{
            return;
        } 
    }
    if(currentCycle>=1&&currentCycle<=256){
        if(currentScanline!=-1&&currentScanline<240){
            for(int i = (currentCycle/8)+2 ; i < 34; i++){
                unsigned char upperTile = upperPattern&0x00FF;
                unsigned char lowerTile = lowerPattern&0x00FF;
                if(cyclesLeft >= 8){
                    addCycles(8);
                }
                else{
                    return;
                }
                for(int patternBit = 0; patternBit < 8; patternBit++){
                    short palleteAddress = 0x3F00 | ((attribute << 2) | ((lowerTile >> 7) << 1 )| (upperTile >> 7));   
                    if((((lowerTile >> 7) << 1 )| ((upperTile >> 7))==0)){
                        palleteAddress = 0x3F00; // check this
                    }
                    upperTile<<=1;
                    lowerTile<<=1; 
                    char palleteIndex = readAddress(palleteAddress);
                    int x = (i-2)*8 + patternBit;
                    int y = currentScanline;
                    printf("RENDERING %d %d\n", x, y);
                    setPixel(x, y, palletes[palleteIndex&0x3F]); // need to refactor
                }

                upperPattern >>= 8;
                lowerPattern >>= 8;
                if(i<32){
                    fetchTile(i);
                }
            }
            renderFlag = true;
        }
        else {
            if(currentScanline>=240){
                if (currentScanline == 241){
                    unsigned char status = getRegister(PPUSTATUS);
                    setRegister(PPUSTATUS, status|0x80);
                    if(shouldInterrupt()){
                        NMI->triggerInterrupt();
                    }    
                }
                if (cyclesLeft>=341){
                    addCycles(341);
                    currentCycle = 0;
                }
                else {
                    return;
                }
            }
        }
        currentScanline+=1;
        if(currentScanline==261){
            currentScanline = -1;
            unsigned char status = getRegister(PPUSTATUS);
            setRegister(PPUSTATUS, status&0x7F);
        }
        // fetch nextScanlineData
    }
    //check for enable rednering
    if (currentCycle==257){
        if(cyclesLeft>=64){
            addCycles(64);
        }
        else{
            return;
        } 
    }
    if (currentCycle==321){
        if (cyclesLeft>=8){
            addCycles(8);
            fetchTile(0);
        }
    }
    if(currentCycle==329){
        if(cyclesLeft>=8){
            addCycles(8);
            fetchTile(1);  
        }
    }
    if(currentCycle==337){
        if (cyclesLeft>=4){
            addCycles(4);
            currentCycle = 0;
        }
    }
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
bool PPU::shouldRender(){
    return renderFlag && currentScanline == 240;
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