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
  std::unique_ptr<Cartridge> cartridge;
  std::shared_ptr<Controller> controller;
  std::shared_ptr<APU> apu;
  std::weak_ptr<PPU> ppu;

  

public:
  Memory(const char *path, std::shared_ptr<Controller> controller, std::shared_ptr<APU>(apu)): cartridge(std::make_unique<Cartridge>(path)),controller(controller), apu(apu) {
  std::fill(memory.begin(), memory.end(), 0);
  // this->ppu = ppu;
  this->controller = controller;
  this->apu = apu;
  };
  void setPPU(std::weak_ptr<PPU> ppu);
  uint8_t readAddress(unsigned short address);
  uint8_t readCHRAddress(unsigned short address);
  void writeCHRAddress(unsigned short addres, uint8_t value);
  void writeAddress(unsigned short address, uint8_t value);
  short readLittleEndian(unsigned short address);
  void OAMDMA(uint8_t data);
};

#endif
