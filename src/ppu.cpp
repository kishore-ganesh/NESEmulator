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
    this->xscroll = 0;
    this->yscroll = 0;
    this->addressLatch = false;
    this->inVblank = false;
    this->internalBuffer = 0;
    setRegister(PPUCTRL, 0);
    setRegister(PPUMASK, 0);
    setRegister(PPUSTATUS, 0xA0);
    setRegister(OAMADDR, 0x0);
    setRegister(PPUADDR, 0);
    
    // memset(internalBuffer, 10240, 0);
    
}

unsigned char PPU::readAddress(unsigned short address, bool external)
{
    // std::cout << address << std::endl;
    address = (address % 0x4000);
    if(address>=0x0000&&address<=0x1FFF){
        if(!external){
            // internalBuffer[address] = memory->readCHRAddress(address);
            return memory->readCHRAddress(address);
        }
        else{
            
            // SPDLOG_INFO("EXTERNAL CHR");
            char value = internalBuffer;
            internalBuffer = memory->readCHRAddress(address);
            return value;
            // return memory->readCHRAddress(address);
        }
        
    }
    if (address >= 0x2000 && address <= 0x2FFF){
        SPDLOG_INFO("Mirroring mode: {}", mirroringMode == Mirroring::VERTICAL);
        if(!external){
            SPDLOG_INFO("PPU READING: {0:x}", address);
        }
        
        switch(mirroringMode){
            case Mirroring::HORIZONTAL: {
                unsigned short baseAddress = address >= 0x2800 ? 0x400 : 0x000;
                unsigned short offset = (address % 0x400);
                if(!external){
                    return vram[baseAddress + offset];
                }
                else{
                    char value = internalBuffer;
                    internalBuffer = vram[baseAddress + offset];
                    return value;
                }
                
                break;
            }
            case Mirroring::VERTICAL: {
                unsigned short baseAddress = ((address < 0x2400) || (address >= 0x2800 && address < 0x2c00) ) ? 0x000: 0x400;
                unsigned short offset = (address % 0x400);
                // SPDLOG_INFO("IN VERTICAL")
                // SPDLOG_INFO("Address: {0:x}, Calculated: {1:x}", address, baseAddress);
                if(!external){
                    // internalBuffer[baseAddress + offset] = vram[baseAddress + offset];
                    return vram[baseAddress + offset];
                }
                else{
                    // return vram[baseAddress + offset];
                    
                    char value = internalBuffer;
                    internalBuffer = vram[baseAddress + offset];
                    return value;
                }
                // return vram[baseAddress + offset];
                break;
            }
        }
        // return vram[address - 0x2000];
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        SPDLOG_INFO("Accessing weird");
        return readAddress(address - 0x1000, external);
    }
    if (address>=0x3F00&&address<=0x3F1F){
        switch(address){
            case 0x3F10: {address = 0x3F00; break;}
            case 0x3F14: {address = 0x3F04; break;}
            case 0x3F18: {address = 0x3F08; break;}
            case 0x3F1c: {address = 0x3F0C; break;}
        }
        return programPalletes[address-0x3F00];
        /*  retyurn pallete */
    }

    if(address >= 0x3F20 && address <= 0x3FFF){
        SPDLOG_INFO("REPLICATED PALLETES");
        return  readAddress((address- 0x3F20) % 0x20 + 0x3F00, external);
        // return programPalletes[address-0x3F00]; // fix this
        /* Return pallete -  */
    }
}

void PPU::writeAddress(unsigned short address, char value){
    address = (address%0x4000);
    if(!inVblank){
        SPDLOG_INFO("WRITING OUTSIDE OF VBLANK");
    }
    if (address >= 0x2000 && address <= 0x2FFF){
        // spdlog::error("WRITE EXCEED");
        // if(address>0x27ff){
        //     spdlog::error("WRITE EXCEED");
        // }
        // vram[address - 0x2000] = value;
        SPDLOG_INFO("Mirroring mode: {}", mirroringMode == Mirroring::VERTICAL);
        switch(mirroringMode){
            case Mirroring::HORIZONTAL: {
                unsigned short baseAddress = address >= 0x2800 ? 0x400 : 0x000;
                unsigned short offset = (address % 0x400);
                vram[baseAddress + offset]  = value;
                break;
            }
            case Mirroring::VERTICAL: {
                unsigned short baseAddress = ((address < 0x2400) || (address >= 0x2800 && address < 0x2c00) ) ? 0x000: 0x400;
                unsigned short offset = (address % 0x400);
                vram[baseAddress + offset] = value;
                break;
            }
        }
        
    }

    if (address >= 0x3000 && address <= 0x3EFF){
        
        spdlog::error("WRITE EXCEED");
        // SPDLOG_INFO("WRITE EXCEED");
        writeAddress(address - 0x1000, value);
        // vram[address-0x3000] = value;
    }

    if (address>=0x3F00&&address<=0x3F1F){
        switch(address){
            case 0x3F10: {address = 0x3F00; break;}
            case 0x3F14: {address = 0x3F04; break;}
            case 0x3F18: {address = 0x3F08; break;}
            case 0x3F1c: {address = 0x3F0C; break;}
        }
        programPalletes[address-0x3F00] = value;
        /*  retyurn pallete */
    }

    if(address >= 0x3F20 && address <= 0x3FFF){
        // spdlog::error("REPLICATED PALLETES");
        // SPDLOG_INFO("REPLICATED PALLETES");
        writeAddress((address- 0x3F20) % 0x20 + 0x3F00, value);
        // programPalletes[(address-0x3F20)%0x20] = value;
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
            addressLatch = false;
            // SPDLOG_INFO("PPUSTATUS READ");
            // address = 0;
            // address = address >
            // xscroll = 0;
            // yscroll = 0;
            break; 
        }
        case OAMDATA: {
            return OAM[value]; 
            break;
        }
        case PPUDATA: {
            SPDLOG_INFO("READING FROM PPU ADDRESS: {0:x}", address);
            value = readAddress(address, true);
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
    // printf("OAM WRITING in %x VALUE: %d ", address, value);
    if(address>255){
        spdlog::error("OAM INVALID ADDRESS");
    }
    OAM[address] = value;
}

void PPU::writeRegister(Registers reg, unsigned char value){
    //Don't need to handle other cases, we're handling here
    SPDLOG_INFO("Register {0:d} set to {1:d}", (int)reg, value);
    if(!inVblank){
        SPDLOG_INFO("WRITING REGISTERS OUTSIDE OF VBLANK");
    }
    if(reg == PPUCTRL && (value&0x03)!=(getRegister(PPUCTRL)&0x03)){
        SPDLOG_INFO("Register PPUCTRL changed");
    }
    setRegister(reg, value);
    unsigned char ppuStatus = getRegister(PPUSTATUS);
    setRegister(PPUSTATUS, (ppuStatus & 0xE0)|(value & 0x1F));
    switch(reg){
        case PPUCTRL: {
            baseAddress = getNameTableAddress(value & 0x03);
            break;
        }
        case OAMADDR: {
            SPDLOG_INFO("OAM ADDRESS SET: {0:x}", value);
            setRegister(OAMADDR, value);
            break;
        }
        case OAMDATA:{
            unsigned char address = getRegister(OAMADDR);
            SPDLOG_INFO("OAM ADDRESS: {0:d}", address);
            writeOAM(address, value);
            setRegister(OAMADDR, address + 1);
            break;
        }
        case PPUSCROLL: {
            // SPDLOG_INFO("PPUSCROLL set");
            if(!addressLatch){
                xscroll = value;
                SPDLOG_INFO("PPU xscroll set to: {0:d} at scanline: {1:d}", xscroll, currentScanline);
                addressLatch = true;
            }
            else{
                yscroll = value;
                // SPDLOG_INFO("PPU yscroll set to: {0:d}", yscroll);
                addressLatch =  false;
            }
            // addressLatch = !addressLatch;
            // unsigned char existingScroll = (scroll&0xFF00) >> 8;
            // scroll = existingScroll | (value << 8);
            // xscroll = existingScroll;
            // yscroll = value; 
            SPDLOG_INFO("Scrolling => x: {0:d}, y: {0:d}", xscroll, yscroll);
            break;
        }
        case PPUADDR: {
            // SPDLOG_INFO("Address latch is: {0:b}", addressLatch);
            
            if(!addressLatch){
                address = (address&0x00FF) | (value<<8);
                addressLatch = true;
                baseAddress = getNameTableAddress((value & 0x0C)>>2);
            }
            else{
                address = (address & 0xFF00) | value;
                addressLatch =   false;
                
            }
            SPDLOG_INFO("Value is: {0:x}, address now: {1:x}", value, address);
            
            // address = (address << 8) | value;
            // address = (address%0x4000);
            SPDLOG_INFO("PPU ADDRESS NOW: {0:x}", address);
            break;
        }
        case PPUDATA: {
            SPDLOG_INFO("WRITING TO PPU DATA: {0:x}", address);
            writeAddress(address, value);
            char increment = getIncrement();
            // setRegister(PPUADDR, address + increment);
            address+=increment;
            break;
        }
    }
}

unsigned short PPU::getNameTableAddress(unsigned char nameTableNumber){
    // char nameTableNumber = getRegister(PPUCTRL) & 0x03;
    short baseAddress = 0x2000;
    short address = baseAddress + ((nameTableNumber*4) << 8);
    SPDLOG_INFO("NAMETABLE ADDRRESS: {0:x}", address);
    SPDLOG_INFO("PPUCTRL: {0:x}", getRegister(PPUCTRL));
    // SPDLOG_INFO("NNO is: {0:d}", nameTableNumber);

    return address; //check
}

short PPU::getBasePatternTableAddress(bool background){
    char mask = 0x10;
    SPDLOG_INFO("PPU CTRL {0:x}", getRegister(PPUCTRL));
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
    short baseNameTableAddress = baseAddress;
    SPDLOG_INFO("Base nametable addess: {0:x},  xScroll: {2:d}, yScroll: {3:d}", baseNameTableAddress, tileNumber, xscroll, yscroll);
    SPDLOG_INFO("Internal address: {0:x}", address);
    if(tileNumber > 31){
        baseNameTableAddress += 0x400;
        tileNumber = tileNumber % 32;
    }
    short nameTableOffset = tileNumber + (currentScanline/8) * 32;
    // SPDLOG_INFO("NAMETABLE ADDRESS: {0:x}", baseNameTableAddress+nameTableOffset);
    unsigned char nameTableEntry = readAddress(baseNameTableAddress + nameTableOffset, false);    
    short basePatternTableAddress = getBasePatternTableAddress(true);
    SPDLOG_INFO("Base pattern address: {0:x}", basePatternTableAddress);
    short baseAttributeTableAddress = baseNameTableAddress + 0x3C0; //check this, make this only one memory acces
    // printf("NAMETABLE OFFSET %d", nameTableOffset);
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
    unsigned char attributeEntry = readAddress(attributeAddress, false);
    
    // short attributeTableAddressOffset = (((attributeNameTableAddress)%16)/2)+(attributeNameTableAddress/32)*8; //Fix calculation
    
    // short attributeTableAddressOffset = (((attributeNameTableAddress)%16)/2)+(attributeNameTableAddress/32)*8; //Fix calculation

    // unsigned char attributeEntry = readAddress(baseAttributeTableAddress + attributeNameTableAddress);
    char r, c;
    c = (tileNumber - xIndex*4)/2;
    r = (currentScanline/8 - yIndex*4)/2;

    // r = ((nameTableOffset%32)/4)/4;

    // r = (nameTableOffset/32) % 4;
    // c = ((nameTableOffset%32))%4;
    SPDLOG_INFO("ROWS: {0:d},  COLUMNS: {1:d}", r, c);
    unsigned char offset = getOffset(r, c);
    attribute = (attributeEntry & (0x03 << (offset*2))) >> (offset*2); // check if this is correct for 2 tiles
    //Multiplying by 16 since each pattern has two consecutive parts (The upper part and the lower part)
    unsigned short patternAddress = nameTableEntry*16 + basePatternTableAddress + (currentScanline%8); // Should not be current scanline
    if(patternAddress > basePatternTableAddress + 0x0FFF){
        SPDLOG_INFO("Pattern address: {0:x}", patternAddress);
    }
    
    upperPattern&=(0x00FF);
    lowerPattern&=(0x00FF);
    upperPattern|=(readAddress(patternAddress, false)) << 8;
    lowerPattern|=(readAddress(patternAddress+8, false)) << 8;
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

    // printf("LINENO: %d, CURRS: %d", lineNo, currentScanline%8);
    if(verticalFlip){
        lineNo = 7 - lineNo;
    }

    // printf("ATTRIBUTES: %d", attribute);
    unsigned short patternAddress = baseSpritePatternAddress + (secondaryOAM[oamIndex].tileIndex * 16) + lineNo;
    unsigned char upperTile = readAddress(patternAddress, false);
    unsigned char lowerTile = readAddress(patternAddress + 8, false);
    struct TileInfo tileInfo = {
        upperTile,
        lowerTile,
        attribute,
        currentScanline,
        secondaryOAM[oamIndex].x,
        horizontalFlip,
        false,
        secondaryOAM[oamIndex].index,
        secondaryOAM[oamIndex].attributes & 0x20
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
        bool isTransparent = false;
        if((tileInfo.lowerPattern>>7)==0&&(tileInfo.upperPattern>>7)==0){
            //Should be mirror
            if(tileInfo.background){
                palleteAddress = 0x3F00;
            }
            else{
                isTransparent = true;
            }
        }
        // SPDLOG_INFO("PALLETE ADDRESS: {0:x}", palleteAddress);
        char palleteIndex = readAddress(palleteAddress, false);
        //Need to evaluate priority here
        unsigned short x = tileInfo.x + patternBit;
        // unsigned char y = secondaryOAM[oamIndex].y;
        if(tileInfo.horizontalFlip){
            x = tileInfo.x + 7 - (patternBit);
        }
        RGB transparentPallete = palletes[readAddress(0x3F00, false)];
        if(!tileInfo.background && tileInfo.spriteIndex == 0  && !isTransparent && !(transparentPallete==getPixel(x, currentScanline))){
            // SPDLOG_INFO("Background is: {}")
            SPDLOG_INFO("SPRITE ZERO HIT at x: {0:d}, scanline: {1:d}", x, currentScanline);
            unsigned char ppuStatus = getRegister(PPUSTATUS);
            setRegister(PPUSTATUS, ppuStatus | 0x40);
        }

        if(tileInfo.background){
            if((x - xscroll%8 )>=0){
                // SPDLOG_INFO("Background x: {0:d}, y: {1:d} is: {2:x}", x-xscroll%8, tileInfo.y, palleteAddress);
                setPixel(x - xscroll%8, currentScanline, palletes[palleteIndex]);
            }
        }
        else{
            // SPDLOG_INFO("x: {0:d}, y: {1:d}, actualX: {2:d}", x, tileInfo.y, tileInfo.x);
            if(x < 256){
                if(!isTransparent && (!tileInfo.priority || transparentPallete==getPixel(x, currentScanline))){
                    // SPDLOG_INFO("SPRITE {}");
                    setPixel(x, currentScanline, palletes[palleteIndex]);
                }
            }
            
        }
        
        tileInfo.upperPattern <<= 1;
        tileInfo.lowerPattern <<= 1;
    }

}

void PPU::generateFrame(int cycles){
    cyclesLeft += cycles;
    SPDLOG_INFO("CURRENT CYCLE: {0:d}, CYCLES LEFT: {1:d}", currentCycle, cyclesLeft);
    int regValue = getRegister(PPUCTRL);
    SPDLOG_INFO("SPRITE MODE: {0}", (regValue & 0x20)?"8x16":"8x8");
    SPDLOG_INFO("SECONDARY OAM SIZE: {0:d}", secondaryOAM.size());
    SPDLOG_INFO("Current scanline: {0:d}, Current scroll: {1:d}, Current Nametable Address: {2:x}", currentScanline, xscroll, baseAddress);
    SPDLOG_INFO("Sprite zero hit: {0:b}", getRegister(PPUSTATUS) & 0x40);
    if(currentCycle==0){
        if(cyclesLeft>=1){
            if(currentScanline==-1){
                unsigned char ppuStatus = getRegister(PPUSTATUS);
                SPDLOG_INFO("Sprite zero cleared");
                setRegister(PPUSTATUS, ppuStatus & ~(0x40));
            }
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
    // SPDLOG_INFO("CURRENT SCANLINE: {0:d}", currentScanline);
    for(int oamIndex = 0; oamIndex < 256; oamIndex+=4){
                //Should be absolute distance
        SPDLOG_INFO("SPRITE OAM: {0:d} {1:d} {2:d}", OAM[oamIndex+3],OAM[oamIndex], currentScanline);
        if((abs(currentScanline - OAM[oamIndex] - 1))<8 && secondaryOAM.size() < 8){
                //Add size check
            secondaryOAM.push_back({OAM[oamIndex]+1, OAM[oamIndex+1], OAM[oamIndex+2], OAM[oamIndex+3], oamIndex});
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
                    char lineNo = currentScanline  - secondaryOAM[oamIndex].y;
                    if(secondaryOAM[oamIndex].y > currentScanline || lineNo > 7 || secondaryOAM[oamIndex].x > (7 + (i-2)*8)){
                           continue;
                    }
                    secondaryOAM[oamIndex].print();

                    // Attribute - Flip
                    // attribute = 0;
                    TileInfo tileInfo = fetchSpriteTile(oamIndex);
                    renderTile(tileInfo);
                    //Implement priority
                }

            //Place it in the right place
    // SPDLOG_INFO("CURRENT SCANLINE: {0:d}", currentScanline);



                //Add OAM[n][M]part

                //Add HBlank here
                upperPattern >>= 8;
                lowerPattern >>= 8;
                if(i<32){
                    fetchTile(i + xscroll/8);
                }
            }


            renderFlag = true;
        }
        else {
            if(currentScanline>=240){
                if (currentScanline == 241){
                    unsigned char status = getRegister(PPUSTATUS);
                    inVblank = true;
                    setRegister(PPUSTATUS, status|0x80);
                    SPDLOG_INFO("SHOULD INTERRUPT: {0:b}, PPUCTRL: {1:d}", shouldInterrupt(), getRegister(PPUCTRL));
                    if(shouldInterrupt()){
                        NMI->triggerInterrupt();
                    }    
                }
                if (cyclesLeft>=341){
                    addCycles(341);
                    // cyclesLeft = 0;
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
        SPDLOG_INFO("CURRENT SCANLINE: {0:d}", currentScanline);
        if(currentScanline==261){
            currentScanline = -1;
            unsigned char status = getRegister(PPUSTATUS);
            setRegister(PPUSTATUS, status&0x7F);
            inVblank = false;
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
            SPDLOG_INFO("NEXT: {0:d}", currentScanline);
            addCycles(8);
            fetchTile(0 + xscroll/8);
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
            fetchTile(1 + xscroll/8);  
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
    // std::vector<std::vector<RGB>> frame = display;
    // RGB background = palletes[readAddress(0x3F00, false)];
    // RGB background = {0, 0,0};
    // for(int x = 0; x < 256; x++){
    //     for(int y = 0; y < 240; y++){
    //         setPixel(x, y, background);
    //     }
    // }
    return display;
}
bool PPU::shouldRender(){
    return renderFlag && currentScanline == 240;
}

void PPU::setMirroringMode(bool mode){
    mirroringMode = mode ? Mirroring::VERTICAL : Mirroring::HORIZONTAL;
}

bool PPU::getCyclesLeft(){
    SPDLOG_INFO("PPU CYCLES LEFT: {0:d}", cyclesLeft);
    // Only effective if we're doing the whole frame here
    return cyclesLeft > 341;
}

unsigned short PPU::getAddress(){
    return address;
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