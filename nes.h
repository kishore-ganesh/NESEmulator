#ifndef NES_INCLUDE
#define NES_INCLUDE
#include "cpu.h"
#include<stdio.h>
#include<iostream>


class NES{
    public:
     //check check setting of stack pointer
    CPU* cpu;
    Memory* memory;
    PPU* ppu;
    int cpuCycles;
    /*
    A - Accumulator
    X, Y - Index register
    P - Processor Status (flag)
     */
    unsigned short PC;
    bool IRQ, NMI;
    bool previousNMILevel;
    NES(char* path);
    void cycle();
    void cpuCycle();
    void ppuCycle();
    bool ppuCyclesLeft();
    bool shouldRender();
    std::vector<std::vector<RGB>> getFrame();
};
#endif