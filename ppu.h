#ifndef PPU_INCLUDE
#define PPU_INCLUDE
#include "memory.h"
#include "interrupt.h"
#include <SDL2/SDL.h>

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

struct RGB{
    unsigned char r, g, b;
};
class PPU{
    char vram[2*1024];
    char registers[8];
    short scroll, address;
    char xscroll, yscroll;
    char OAM[256];
    RGB display [256][240]; //take care of x and y
    Memory* memory;
    SDL_Window* window;
    SDL_Renderer* renderer;
    EdgeInterrupt* NMI;
    /*
    These are palletes used by the NES (taken verbatim from Blargg's palletes). Later we will add NTSC decoding to mirror what the NES actually
    does
     */
    RGB palletes[64] = {
        { 84, 84, 84 },
        { 0, 30, 116 },
        { 8, 16, 144 },
        { 48, 0, 136 },
        { 68, 0, 100 },
        { 92, 0, 48 },
        { 84, 4, 0 },
        { 60, 24, 0 },
        { 32, 42, 0 },
        { 8, 58, 0 },
        { 0, 64, 0 },
        { 0, 60, 0 },
        { 0, 50, 60 },
        { 0, 0, 0 },
        { 152, 150, 152 },
        { 8, 76, 196 },
        { 48, 50, 236 },
        { 92, 30, 228 },
        { 136, 20, 176 },
        { 160, 20, 100 },
        { 152, 34, 32 },
        { 120, 60, 0 },
        { 84, 90, 0 },
        { 40, 114, 0 },
        { 8, 124, 0 },
        { 0, 118, 40 },
        { 0, 102, 120 },
        { 0, 0, 0 },
        { 236, 238, 236 },
        { 76, 154, 236 },
        { 120, 124, 236 },
        { 176, 98, 236 },
        { 228, 84, 236 },
        { 236, 88, 180 },
        { 236, 106, 100 },
        { 212, 136, 32 },
        { 160, 170, 0 },
        { 116, 196, 0 },
        { 76, 208, 32 },
        { 56, 204, 108 },
        { 56, 180, 204 },
        { 60, 60, 60 },
        { 236, 238, 236 },
        { 168, 204, 236 },
        { 188, 188, 236 },
        { 212, 178, 236 },
        { 236, 174, 236 },
        { 236, 174, 212 },
        { 236, 180, 176 },
        { 228, 196, 144 },
        { 204, 210, 120 },
        { 180, 222, 120 },
        { 168, 226, 144 },
        { 152, 226, 180 },
        { 160, 214, 228 },
        { 160, 162, 160 }
    };
    public:
    PPU(Memory* memory, EdgeInterrupt* NMI);
    char readAddress(unsigned short address);
    void writeAddress(unsigned short address, char value);
    char getRegister(Registers reg);
    void setRegister(Registers reg, char value);
    char getIncrement();
    /*
    Read register is used when CPU reads something from PPU. It is
    an operation, unlike get register, which just gets the register without doing anything PPU specific
     */
    char readRegister(Registers reg);
    void writeRegister(Registers reg, char value);
    short getNameTableAddress();
    short getBasePatternTableAddress(bool background);
    void cycle();
    RGB getPixel(int x, int y);
    void setPixel(int x, int y, RGB value);
    void generateFrame();
    void displayFrame();
    
};

/*
PPU has both RAM 

 */

#endif