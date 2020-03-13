#ifndef CARTRIDGE_INCLUDE
#define CARTRIDGE_INCLUDE
#include "unrom_mapper.h"
#include "util.h"
struct iNES_Header{
    char NESConstant[4];
    char prgSize;
    char chrSize;
    char flag[5];
    char zero[5];
};

enum class Mapper{
    NROM,
    MMC1,
    UNROM
};
class Cartridge {
    char* PRG_ROM;
    char CHR_ROM[8192]; //check this
    UNROM* unrom;
    Mapper type;
    iNES_Header header;
    public:
    Cartridge(char* path);
    unsigned char readPRGAddress(unsigned short address);
    unsigned char readCHRAddress(unsigned short address);
    void writeCHRAddress(unsigned short address, unsigned char value);
    void write(unsigned short address, char value);
    void printHeader(iNES_Header header);
    bool getMirroringMode();
};

#endif