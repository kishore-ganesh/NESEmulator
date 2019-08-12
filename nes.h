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
    unsigned char display [256][240]; //take care of x and y
    NES(char* path);
    void start();
};
