#ifndef PPU_INCLUDE
#define PPU_INCLUDE
#include "memory.h"
#include "interrupt.h"
#include "util.h"
#include<vector>

//Nameppaces

class Memory; // why did forward declaration work
enum Registers{
    PPUCTRL,
    PPUMASK,
    PPUSTATUS,
    OAMADDR,
    OAMDATA,
    PPUSCROLL,
    PPUADDR,
    PPUDATA,
};


struct Sprite{
    unsigned char y;
    unsigned char tileIndex;
    unsigned char attributes;
    unsigned char x;
    void print(){
        spdlog::info("SPRITE at {0:d}, {1:d} with tileIndex {2:d}", x, y, tileIndex);
    }
};

struct SpritePPUInfo{
    unsigned char upperPattern;
    unsigned char lowerPattern;
    unsigned char latch;
    unsigned char xPosition;
};

struct TileInfo{
    unsigned char upperPattern;
    unsigned char lowerPattern;
    unsigned char attribute;
    int y;
    int x;
    bool horizontalFlip;
    bool background;

};
class PPU
{
    bool renderFlag;
    unsigned char vram[2 * 1024];
    char registers[8];
    short scroll;
    unsigned short upperPattern, lowerPattern;
    unsigned char attribute;
    unsigned short address;
    char xscroll, yscroll;
    unsigned char OAM[256];
    std::vector<Sprite> secondaryOAM;
    int cyclesLeft;
    int cyclesNeeded;
    int currentCycle;
    int currentScanline;

    std::vector<std::vector<RGB>> display = std::vector<std::vector<RGB>>(256, std::vector<RGB>(240));; //take care of x and y
    Memory *memory;
    EdgeInterrupt *NMI;
    /*
    These are palletes used by the NES (taken verbatim from Blargg's palletes). Later we will add NTSC decoding to mirror what the NES actually
    does
     */

    RGB palletes[64] = {
        {124, 124, 124},
        {0, 0, 252},
        {0, 0, 188},
        {68, 40, 188},
        {148, 0, 132},
        {168, 0, 32},
        {168, 16, 0},
        {136, 20, 0},
        {80, 48, 0},
        {0, 120, 0},
        {0, 104, 0},
        {0, 88, 0},
        {0, 64, 88},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {188, 188, 188},
        {0, 120, 248},
        {0, 88, 248},
        {104, 68, 252},
        {216, 0, 204},
        {228, 0, 88},
        {248, 56, 0},
        {228, 92, 16},
        {172, 124, 0},
        {0, 184, 0},
        {0, 168, 0},
        {0, 168, 68},
        {0, 136, 136},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {248, 248, 248},
        {60, 188, 252},
        {104, 136, 252},
        {152, 120, 248},
        {248, 120, 248},
        {248, 88, 152},
        {248, 120, 88},
        {252, 160, 68},
        {248, 184, 0},
        {184, 248, 24},
        {88, 216, 84},
        {88, 248, 152},
        {0, 232, 216},
        {120, 120, 120},
        {0, 0, 0},
        {0, 0, 0},
        {252, 252, 252},
        {164, 228, 252},
        {184, 184, 248},
        {216, 184, 248},
        {248, 184, 248},
        {248, 164, 192},
        {240, 208, 176},
        {252, 224, 168},
        {248, 216, 120},
        {216, 248, 120},
        {184, 248, 184},
        {184, 248, 216},
        {0, 252, 252},
        {248, 216, 248},
        {0, 0, 0},
        {0, 0, 0},
    };
    char programPalletes[32];

public:
    PPU(Memory *memory, EdgeInterrupt *NMI);
    unsigned char readAddress(unsigned short address);
    void writeAddress(unsigned short address, char value);
    unsigned char getRegister(Registers reg);
    void setRegister(Registers reg, char value);
    char getIncrement();
    void addCycles(int cycles);
    void addCPUCycles(int cycles);
    void fetchTile(int tileNumber);
    TileInfo fetchSpriteTile(int oamIndex);
    void renderTile(TileInfo tileInfo);
    /*
    Read register is used when CPU reads something from PPU. It is
    an operation, unlike get register, which just gets the register without doing anything PPU specific
     */
    unsigned char readRegister(Registers reg);
    void writeRegister(Registers reg, unsigned char value);
    void writeOAM(unsigned char address, unsigned char value);
    unsigned short getNameTableAddress();
    short getBasePatternTableAddress(bool background);
    bool shouldInterrupt();
    void cycle();
    RGB getPixel(int x, int y);
    std::vector<std::vector<RGB>> getFrame();
    void setPixel(int x, int y, RGB value);
    void generateFrame(int cycles);
    void displayFrame();
    bool shouldRender();
    bool getCyclesLeft();
};

/*
PPU has both RAM 

 */

#endif