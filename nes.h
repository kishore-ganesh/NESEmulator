#include "cpu.h"
#include<stdio.h>
#include<iostream>


class NES{
    public:
     //check check setting of stack pointer
    CPU* cpu;
    Memory* memory;
    PPU* ppu;
    
    /*
    A - Accumulator
    X, Y - Index register
    P - Processor Status (flag)
     */
    unsigned short PC;
    bool IRQ, NMI;
    bool previousNMILevel;
    NES(char* path);
    void start();
};
