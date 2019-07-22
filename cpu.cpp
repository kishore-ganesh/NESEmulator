#include<stdio.h>
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
struct iNES_Header{
    char NESConstant[4];
    char prgSize;
    char chrSize;
    char flag[5];
    char zero[4];
};
class NES{
    char A, X, Y, P, SP; //check this

    /*
    A - Accumulator
    X, Y - Index register
    P - Processor Status (flag)
     */
    unsigned short PC;
    char memory[2*1024];
    char PRG_ROM[32768];
    unsigned char display [256][240]; //take care of x and y
    public: 
    NES(){
        PC = 0x8000;
        P = 0x34;
    }
    char readAddress(unsigned short address){
        if(address<=0x1FFF){
            return memory[address%(0x0800)];
        }
        else if(address>=0x8000&&address<=0xFFFF){
            return PRG_ROM[address];
        }
    }
    void readFile(char* path){
        iNES_Header header;
        FILE* rom = fopen(path, "r");
        fread(&header, sizeof(header), 1, rom);
        if(header.flag[0]&0x0004){
            fseek(rom, 512, SEEK_CUR); //check this
        }
        
        long pos = ftell(rom);
        fread(PRG_ROM, header.prgSize*0x4000, 1, rom);
        if(header.prgSize==1){
            fseek(rom, pos, SEEK_SET);
            fread(&PRG_ROM[0x4000], 0x4000, 1, rom);
        }
    }
    void cycle(){
        char instruction = readAddress(PC);
        
        PC = PC + 1;
    }
    void ORA(unsigned short address){
        A = A | memory[address];
    }

    void AND(unsigned short address){
        A = A & memory[address];
    }
    void EOR(unsigned short address){
        A = A ^ memory[address];
    }
    void ADC(unsigned short address){
        A = A + memory[address];
    }
    void SBC(unsigned short address){
        A = A - memory[address];
    }
};

int main(int argc, char* argv[]){
    NES nes;
    nes.readFile(argv[1]);
}