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
    short readLittleEndian(unsigned short address){
        short data = 0;
        data = readAddress(address+1);
        data = (data)<<4;
        data = data | readAddress(address);
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
    void processInstruction(char instruction){
        
        if(instruction&0x01){
            char data = 0;
            short address;
            /* Check addressing modes */
            switch(instruction&0x1E0){
                case 0x0: {
                    PC = PC + 1;
                    address = readLittleEndian(PC+X);
                    PC = PC + 1;
                    data = readAddress(address);
                    break;
                } 
                case 0x1: {
                    PC = PC + 1;
                    data = readAddress(PC);
                    data = readAddress(data);
                    break;
                }
                case 0x2:{
                    PC = PC + 1;
                    data = readAddress(PC);
                    break;
                }
                case 0x3: {
                    short address;
                    PC = PC + 1;
                    address = readLittleEndian(PC);
                    PC = PC + 1;
                    data = readAddress(address);
                    break;
                }
                case 0x4: {
                    PC = PC + 1;
                    short address = readLittleEndian(PC);
                    PC = PC + 1;
                    data = readAddress(address+Y);
                    break;
                }
                case 0x5: {
                    PC = PC + 1;
                    data = readAddress(PC+X);
                }
                case 0x6: { 
                    PC = PC + 1;
                    short int address;
                    address = readLittleEndian(PC);
                    PC = PC + 1;
                    data = readAddress(address+X);
                    break;
                }
                case 0x7: {
                    short int address;
                    PC = PC + 1;
                    address = readLittleEndian(PC);
                    PC = PC + 1;
                    data = readAddress(address+Y);
                    break;
                }
            }

            switch(instruction&0xE0){
                case 0x0: ORA(data); break;
                case 0x1: AND(data); break;
                case 0x2: EOR(data); break; 
                case 0x3: ADC(data); break;
                case 0x4: STA(address); PC = PC -1; break;  
                case 0x5: LDA(data); break;
                case 0x6: CMP(data); break;
                case 0x7: SBC(data); break;
            }
        }
        
    }
    void cycle(){
        char instruction = readAddress(PC);
        processInstruction(instruction);
        PC = PC + 1;
    }
    void ORA(char data){
        A|=data;
    }

    void AND(char data){
        A&=data;
    }
    void EOR(char data){
        A^=data;
    }
    void ADC(char data){
        if(A+data>0xFF){
            
        }
        

        A+=data;
    }
    void SBC(char data){
        A-=data;
    }
    void STA(short int address){
        memory[address] = A; // check this
    }
    void LDA(char data){
        A = data;
    }
    void CMP(char data){
        // Update flags based on CMP
    }
};

int main(int argc, char* argv[]){
    NES nes;
    nes.readFile(argv[1]);
}