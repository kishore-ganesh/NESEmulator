#ifndef APU_INCLUDE
#define APU_INCLUDE
#include "SDL2/SDL.h"
#include "util.h"

class PulseGenerator{
    unsigned short time;
    unsigned short timer;
    unsigned char dutyCycle;
    unsigned char volume;
    unsigned char length;
    unsigned char sweepUnit;
    unsigned char sweepPeriod;
    unsigned char currentSweepPeriod;
    bool lengthCounterHalt, volumeFlag;
    unsigned char currentSequence;
    int currentSequenceIndex;
    public:
    void writeRegister(unsigned short address, unsigned char value);
    unsigned char cycle();
    bool getCyclesLeft();
    void sweep();
};

class TriangleGenerator{
    public:
    void writeRegister(unsigned short address, unsigned char value);
};

class APUSweep{
    
};

class APU{
    int cyclesLeft;
    int currentCycle;
    int samplesIndex;
    SDL_AudioDeviceID dev;
    unsigned char status;
    unsigned char frameCounter;
    PulseGenerator pulse1, pulse2;
    TriangleGenerator triangle;
    char samples[8192];
    enum class EnableMasks{
        DMC = 0x10,
        NOISE = 0x08,
        TRIANGLE = 0x04,
        PULSE2 = 0x02,
        PULSE1 = 0x01
        

    };

    public:
    APU();
    bool getCyclesLeft();
    void addCPUCycles(char cpuCycles);
    unsigned char readRegister(unsigned short address);
    void writeRegister(unsigned short address, unsigned char value);
    void cycle();
};



// class 

#endif