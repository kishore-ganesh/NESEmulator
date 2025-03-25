#ifndef UNROM_INCLUDE
#define UNROM_INCLUDE
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
class UNROM {
  uint8_t PRG_ROM_Bank[8][16 * 1024];
  uint8_t currentBank;
  uint8_t permanentBank;
  uint8_t CHR_RAM[8 * 1024];

public:
  UNROM(FILE *rom) {

    // Make this dynamic later
    fread(PRG_ROM_Bank, 8 * 0x4000, 1, rom);
    currentBank = 0;
    permanentBank = 7;
  }
  uint8_t readAddress(unsigned short address);

  void writeAddress(unsigned short address, uint8_t value);
  uint8_t readCHRAddress(unsigned short address);
  void writeCHRAddress(unsigned short address, uint8_t value);
};

#endif
