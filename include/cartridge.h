#ifndef CARTRIDGE_INCLUDE
#define CARTRIDGE_INCLUDE
#include "unrom_mapper.h"
#include "util.h"
struct iNES_Header {
  uint8_t NESConstant[4];
  uint8_t prgSize;
  uint8_t chrSize;
  uint8_t flag[5];
  uint8_t zero[5];
};

enum class Mapper { NROM, MMC1, UNROM };
class Cartridge {
  std::vector<uint8_t> PRG_ROM;
  std::array<uint8_t, 8192> CHR_ROM;
  UNROM *unrom;
  Mapper type;
  iNES_Header header;

public:
  Cartridge(char *path);
  uint8_t readPRGAddress(unsigned short address);
  uint8_t readCHRAddress(unsigned short address);
  void writeCHRAddress(unsigned short address, uint8_t value);
  void write(unsigned short address, uint8_t value);
  void printHeader(iNES_Header header);
  bool getMirroringMode();
};

#endif