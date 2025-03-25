#ifndef APU_INCLUDE
#define APU_INCLUDE
#include "SDL2/SDL.h"
#include "util.h"

class PulseGenerator {
  unsigned short time;
  unsigned short timer;
  uint8_t dutyCycle;
  uint8_t volume;
  uint8_t length;
  uint8_t sweepUnit;
  uint8_t sweepPeriod;
  uint8_t currentSweepPeriod;
  bool lengthCounterHalt, volumeFlag;
  uint8_t currentSequence;
  int currentSequenceIndex;

public:
  void writeRegister(unsigned short address, uint8_t value);
  unsigned short cycle();
  void sweep();
};

class TriangleGenerator {

  uint8_t sequence[32] = {15, 14, 13, 12, 11, 10, 9,  8,  7,  6, 5,
                                4,  3,  2,  1,  0,  0,  1,  2,  3,  4, 5,
                                6,  7,  8,  9,  10, 11, 12, 13, 14, 15};
  int currentSequenceIndex;
  unsigned short timer;
  unsigned short time;
  uint8_t length;
  uint8_t linearCounter;
  uint8_t reloadValue;
  bool linearCounterReload;
  bool controlFlag;

public:
  void writeRegister(unsigned short address, uint8_t value);
  unsigned short cycle();
  void linear();
  void lengthCounter();
};

class APUSweep {};

class APU {
  int cyclesLeft;
  int currentCycle;
  int samplesIndex;
  int sample;
  SDL_AudioDeviceID dev;
  uint8_t status;
  uint8_t frameCounter;
  PulseGenerator pulse1, pulse2;
  TriangleGenerator triangle;
  unsigned short samples[8192];
  enum class EnableMasks {
    DMC = 0x10,
    NOISE = 0x08,
    TRIANGLE = 0x04,
    PULSE2 = 0x02,
    PULSE1 = 0x01

  };

public:
  APU();
  bool getCyclesLeft();
  void addCPUCycles(uint8_t cpuCycles);
  uint8_t readRegister(unsigned short address);
  void writeRegister(unsigned short address, uint8_t value);
  void cycle();
};

// class

#endif