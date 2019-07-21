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
    unsigned short PC;
    char memory[2*1024];
    unsigned char display [256][240]; //take care of x and y
    public: 
    void readFile(char* path){
        iNES_Header header;
        FILE* rom = fopen(path, "r");
        fread(&header, sizeof(header), 1, rom);

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