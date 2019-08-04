struct iNES_Header{
    char NESConstant[4];
    char prgSize;
    char chrSize;
    char flag[5];
    char zero[5];
};

class Cartridge {
    char PRG_ROM[32768];
    public:
    Cartridge(char* path);
    char read(unsigned short address);
    void write(unsigned short address, char value);
};