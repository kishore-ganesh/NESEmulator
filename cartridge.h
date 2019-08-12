struct iNES_Header{
    char NESConstant[4];
    char prgSize;
    char chrSize;
    char flag[5];
    char zero[5];
};

class Cartridge {
    char PRG_ROM[32768];
    char CHR_ROM[8192]; //check this
    public:
    Cartridge(char* path);
    unsigned char read(unsigned short address);
    void write(unsigned short address, char value);
};