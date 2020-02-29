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
    this->cyclesNeeded = 0;
    this->currentCycle = 0;
    this->currentScanline = -1;
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
        printf("REPLICATED PALLETES\n");
        return  programPalletes[(address- 0x3520) % 0x20];
        // return programPalletes[address-0x3F00]; // fix this
        /* Return pallete -  */
    }
}

void PPU::writeAddress(unsigned short address, char value){
    address = (address%0x4000);
    if (address >= 0x2000 && address <= 0x2FFF){
        if(address>0x23ff){
            printf("WRITE EXCEED\n");
        }
        vram[address - 0x2000] = value;
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        
        printf("WRITE EXCEED\n");
        vram[address-0x3000] = value;
    }

    if (address>=0x3F00&&address<=0x3F1F){
        programPalletes[address-0x3F00] = value;
        /*  retyurn pallete */
    }

    if(address >= 0x3520 && address <= 0x3FFF){
        printf("REPLICATED PALLETES\n");
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
    if(getRegister(PPUCTRL)&0x04){
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
            break;
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

void PPU::writeOAM(unsigned char address, unsigned char value){
    // printf("OAM WRITING in %x VALUE: %d\n ", address, value);
    if(address>255){
        printf("OAM INVALID ADDRESS\n");
    }
    OAM[address] = value;
}

void PPU::writeRegister(Registers reg, unsigned char value){
    //Don't need to handle other cases, we're handling here
    setRegister(reg, value);
    switch(reg){
        case OAMADDR: {
            printf("OAM ADDRESS SET: %x\n", value);
            setRegister(PPUADDR, value);
            break;
        }
        case OAMDATA:{
            unsigned char address = getRegister(OAMADDR);
            printf("OAM ADDRESS: %d\n", address);
            writeOAM(address, value);
            setRegister(OAMADDR, address + 1);
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

unsigned short PPU::getNameTableAddress(){
    char nameTableNumber = getRegister(PPUCTRL) & 0x03;
    short baseAddress = 0x2000;
    short address = baseAddress + ((nameTableNumber*4) << 8);
    printf("NAMETABLE ADDRRESS: %x\n", address);
    return address; //check
}

short PPU::getBasePatternTableAddress(bool background){
    char mask = 0x10;
    printf("PPU CTRL %x\n", getRegister(PPUCTRL));
    if(!background){
        mask = 0x08;
        // return 0x0000;
    }

    if(!(getRegister(PPUCTRL)&mask)){
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
    //Throw an exception
    currentCycle+=cycles;
    cyclesLeft-=cycles;
}

void PPU::addCPUCycles(int cycles){
    cyclesLeft+=(cycles*3);
}

void PPU::fetchTile(int tileNumber){
    // Fix this
    // 
    short baseNameTableAddress = getNameTableAddress();
    short nameTableOffset = tileNumber + (currentScanline/8) * 32;
    printf("NAMETABLE ADDRESS: %x\n", baseNameTableAddress+nameTableOffset);
    unsigned char nameTableEntry = readAddress(baseNameTableAddress + nameTableOffset);    
    short basePatternTableAddress = getBasePatternTableAddress(true);
    short baseAttributeTableAddress = baseNameTableAddress + 0x3C0; //check this, make this only one memory acces
    // printf("NAMETABLE OFFSET %d\n", nameTableOffset);
    //Check this again
    // short attributeNameTableAddress = (nameTableOffset % 32)/4 + (nameTableOffset/128)*8;
    // index = tileNumber/4, currentScanline/4
    // (index-1)*8 + currentScanline/4
    //That's where attribute from
    //Offset is equal to (t-x*4)/4, currentScanline - (currentScanline/4)*4
    short xIndex = tileNumber/4;
    short yIndex = (currentScanline/8)/4;
    unsigned short attributeOffset = (yIndex*8) + xIndex;
    unsigned short attributeAddress = baseAttributeTableAddress + attributeOffset;
    unsigned char attributeEntry = readAddress(attributeAddress);
    
    // short attributeTableAddressOffset = (((attributeNameTableAddress)%16)/2)+(attributeNameTableAddress/32)*8; //Fix calculation
    
    // short attributeTableAddressOffset = (((attributeNameTableAddress)%16)/2)+(attributeNameTableAddress/32)*8; //Fix calculation

    // unsigned char attributeEntry = readAddress(baseAttributeTableAddress + attributeNameTableAddress);
    char r, c;
    c = (tileNumber - xIndex*4)/2;
    r = (currentScanline/8 - yIndex*4)/2;

    // r = ((nameTableOffset%32)/4)/4;

    // r = (nameTableOffset/32) % 4;
    // c = ((nameTableOffset%32))%4;
    printf("ROWS: %d,  COLUMNS: %d\n", r, c);
    unsigned char offset = getOffset(r, c);
    attribute = (attributeEntry & (0x03 << (offset*2))) >> (offset*2); // check if this is correct for 2 tiles
    //Multiplying by 16 since each pattern has two consecutive parts (The upper part and the lower part)
    unsigned short patternAddress = nameTableEntry*16 + basePatternTableAddress + (currentScanline%8); // Should not be current scanline
    upperPattern&=(0x00FF);
    lowerPattern&=(0x00FF);
    upperPattern|=(readAddress(patternAddress)) << 8;
    lowerPattern|=(readAddress(patternAddress+8)) << 8;
}

TileInfo PPU::fetchSpriteTile(int oamIndex){
    char lineNo = currentScanline - secondaryOAM[oamIndex].y;
    // printf("")
    
    unsigned short baseSpritePatternAddress = getBasePatternTableAddress(false);
    //Modify for 8x16
    unsigned char attribute = secondaryOAM[oamIndex].attributes & 0x03;
    bool verticalFlip = secondaryOAM[oamIndex].attributes & 0x80;
    bool horizontalFlip = secondaryOAM[oamIndex].attributes & 0x40;
    // char lineNo = currentScanline - secondaryOAM[oamIndex].y;

    // printf("LINENO: %d, CURRS: %d\n", lineNo, currentScanline%8);
    if(verticalFlip){
        lineNo = 7 - lineNo;
    }

    // printf("ATTRIBUTES: %d\n", attribute);
    unsigned short patternAddress = baseSpritePatternAddress + (secondaryOAM[oamIndex].tileIndex * 16) + lineNo;
    unsigned char upperTile = readAddress(patternAddress);
    unsigned char lowerTile = readAddress(patternAddress + 8);
    struct TileInfo tileInfo = {
        upperTile,
        lowerTile,
        attribute,
        currentScanline,
        secondaryOAM[oamIndex].x,
        horizontalFlip,
        false
    };
    return tileInfo;
}
void PPU::renderTile(TileInfo tileInfo){
    //Use std::move
    unsigned short baseAddress = 0x3F00;
    if(!tileInfo.background){
        baseAddress = 0x3F10;
    }
    for(int patternBit = 0; patternBit < 8;  patternBit++){   
        unsigned short palleteAddress = baseAddress | (tileInfo.attribute << 2) | ((tileInfo.lowerPattern >> 7)<<1) | (tileInfo.upperPattern>>7);
        if((tileInfo.lowerPattern>>7)==0&&(tileInfo.upperPattern>>7)==0){
            //Should be mirror
            palleteAddress = 0x3F00;
        }
        printf("PALLETE ADDRESS: %x\n", palleteAddress);
        char palleteIndex = readAddress(palleteAddress);
        //Need to evaluate priority here
        unsigned char x = tileInfo.x + patternBit;
        // unsigned char y = secondaryOAM[oamIndex].y;
        if(tileInfo.horizontalFlip){
            x = tileInfo.x + 7 - (patternBit);
        }
        
        setPixel(x, tileInfo.y, palletes[palleteIndex]);
        tileInfo.upperPattern <<= 1;
        tileInfo.lowerPattern <<= 1;
    }

}

void PPU::generateFrame(int cycles){
    cyclesLeft += cycles;
    printf("CURRENT CYCLE: %d, CYCLES LEFT: %d\n", currentCycle, cyclesLeft);
    int regValue = readRegister(PPUCTRL);
    printf("SPRITE MODE: %s\n", (regValue & 0x20)?"8x16":"8x8");
    printf("SECONDARY OAM SIZE: %d\n", secondaryOAM.size());

    if(currentCycle==0){
        if(cyclesLeft>=1){
            addCycles(1);
            renderFlag = false;
        }
        else{
            cyclesNeeded = 1;
            return;
        }  
    }
    secondaryOAM.clear();

            //Place it in the right place
    for(int oamIndex = 0; oamIndex < 256; oamIndex+=4){
                    //Should be absolute distance
        printf("SPRITE OAM: %d %d\n", OAM[oamIndex], OAM[oamIndex+3]);
        if((abs(currentScanline-OAM[oamIndex]))<8 && secondaryOAM.size() < 8){
                    //Add size check
            secondaryOAM.push_back({OAM[oamIndex], OAM[oamIndex+1], OAM[oamIndex+2], OAM[oamIndex+3]});
        }
    }
    if(currentCycle>=1&&currentCycle<=256){
        if(currentScanline!=-1&&currentScanline<240){
            //Current Cycle / 8 + 2: 32 * 8 => 256 cycles here
            for(int i = (currentCycle/8)+2 ; i < 34; i++){
                unsigned char upperTile = upperPattern&0x00FF;
                unsigned char lowerTile = lowerPattern&0x00FF;
                if(cyclesLeft >= 8){
                    addCycles(8);
                }
                else{
                    cyclesNeeded = cyclesLeft - 8;
                    return;
                }

                struct TileInfo tileInfo = {
                    upperTile,
                    lowerTile,
                    attribute,
                    currentScanline,
                    (i-2)*8,
                    false,
                    true
                };

                renderTile(tileInfo);
            

                for(int oamIndex = 0; oamIndex < secondaryOAM.size(); oamIndex++){
                    char lineNo = currentScanline - secondaryOAM[oamIndex].y;
                    if(secondaryOAM[oamIndex].y > currentScanline || lineNo > 7){
                           continue;
                    }
                    secondaryOAM[oamIndex].print();

                    // Attribute - Flip
                    // attribute = 0;
                    TileInfo tileInfo = fetchSpriteTile(oamIndex);
                    renderTile(tileInfo);
                    //Implement priority
                }



                //Add OAM[n][M]part

                //Add HBlank here
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
                    cyclesNeeded = cyclesLeft - 341;
                    return;
                }
            }
        }
        //Are we reaching hee?
        if(currentScanline!=-1){
            currentScanline++;
        }
        else{
            if(cyclesLeft>=320){
                addCycles(320);
                currentScanline++;
            }
            else{
                cyclesNeeded = cyclesLeft - 320;
                return; 
            }
        }
        // currentScanline+=1;
        printf("CURRENT SCANLINE: %d\n", currentScanline);
        if(currentScanline==261){
            currentScanline = -1;
            unsigned char status = getRegister(PPUSTATUS);
            setRegister(PPUSTATUS, status&0x7F);
        }
        // fetch nextScanlineData
    }
    //check for enable rednering
    if(currentCycle>=257 && currentCycle<=320){
        setRegister(OAMADDR, 0);
    }
    if (currentCycle==257){
        if(cyclesLeft>=64){
            addCycles(64);
        }
        else{
            cyclesNeeded = cyclesLeft - 64;
            return;
        } 
    }
    if (currentCycle==321){
        if (cyclesLeft>=8){
            printf("NEXT: %d\n", currentScanline);
            addCycles(8);
            fetchTile(0);
        }
        else{
            cyclesNeeded = cyclesLeft - 8;
            return; 
        }
    }
    if(currentCycle==329){
        if(cyclesLeft>=8){
            upperPattern >>= 8;
            lowerPattern >>= 8;
            addCycles(8);
            fetchTile(1);  
        }
        else{
            cyclesNeeded = cyclesLeft - 8;
             return; 
        }
    }
    if(currentCycle==337){
        if (cyclesLeft>=4){
            addCycles(4);
            currentCycle = 0;
        }
        else{
            cyclesNeeded = cyclesLeft - 4;
            return;
        }
    }
    cyclesNeeded = 0;
    return;
}

RGB PPU::getPixel(int x, int y){
    return display[x][y];
}

void PPU::setPixel(int x, int y, RGB value){
    // std::cout << x << " " << y << std::endl;
    display[x][y] = value;
}

std::vector<std::vector<RGB>> PPU::getFrame(){
    return display;
}
bool PPU::shouldRender(){
    return renderFlag && currentScanline == 240;
}

bool PPU::getCyclesLeft(){
    printf("PPU CYCLES LEFT: %d\n", cyclesLeft);
return cyclesLeft > 30000;
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