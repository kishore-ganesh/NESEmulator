#ifndef APU_INCLUDE
#define APU_INCLUDE
#include "SDL2/SDL.h"
#include "util.h"



class APU{
    int cyclesLeft;
    SDL_AudioDeviceID dev;
    public:
    APU();
    bool getCyclesLeft();
    unsigned char readRegister(unsigned short address);
    void writeRegister(unsigned short address, char value);
};

class PulseGenerator{
    unsigned short period;
    unsigned char dutyCycle;
    unsigned char volume;
};

class TriangleGenerator{

};

// class 

#endif