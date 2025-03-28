#ifndef MEMORY_INCLUDE
#define MEMORY_INCLUDE
#include "apu.h"
#include "cartridge.h"
#include "controller.h"
#include "ppu.h"
#include <iostream>
class PPU;
/*
Class for common memory bus operations ( this is shared between CPU, PPU and
APU)
 */
class Memory {
  std::array<uint8_t, 2 * 1024> memory;
  PPU *ppu;
  std::unique_ptr<Cartridge> cartridge;
  Controller* controller;
  APU *apu;

public:
  Memory(char *path, Controller *controller, APU *apu);
  void setPPU(PPU *ppu);
  uint8_t readAddress(unsigned short address);
  uint8_t readCHRAddress(unsigned short address);
  void writeCHRAddress(unsigned short addres, uint8_t value);
  void writeAddress(unsigned short address, uint8_t value);
  short readLittleEndian(unsigned short address);
  void OAMDMA(uint8_t data);
};

#endif
