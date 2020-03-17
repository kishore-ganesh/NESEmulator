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
    unsigned short cycle();
    void sweep();
};

class TriangleGenerator{

    unsigned char sequence[32] = {15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15};
    int currentSequenceIndex;
    unsigned short timer;
    unsigned short time;
    unsigned char length;
    unsigned char linearCounter;
    unsigned char reloadValue;
    bool linearCounterReload;
    bool controlFlag;
    public:
    void writeRegister(unsigned short address, unsigned char value);
    unsigned short cycle();
    void linear();
    void lengthCounter();
};

class APUSweep{
    
};

class APU{
    int cyclesLeft;
    int currentCycle;
    int samplesIndex;
    int sample;
    SDL_AudioDeviceID dev;
    unsigned char status;
    unsigned char frameCounter;
    PulseGenerator pulse1, pulse2;
    TriangleGenerator triangle;
    unsigned short samples[8192];
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