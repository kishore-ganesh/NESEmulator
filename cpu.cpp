#include<stdio.h>
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
/*We should handle addresses automaitcally or not? */
//Check write addresses
struct iNES_Header{
    char NESConstant[4];
    char prgSize;
    char chrSize;
    char flag[5];
    char zero[4];
};
class NES{
    char A, X, Y, P, SP; //check this
    enum masks{
        CARRY = 0x01,
        ZERO = 0x02,
        INT = 0x04,
       // DEC = 0x08,
        B = 0x10,
        OVERFLOW = 0x20,
        NEGATIVE = 0x40
    } flagMask;
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

    void setFlag(char mask, bool bit){
        P&=!mask;
        if(bit){
            P|=mask;
        }
    }

    void checkValueFlags(char value){
        bool negBit, zeroBit;
        negBit = value < 0 ? 1: 0;
        zeroBit = value = 0? 0: 1;
        setFlag(ZERO, zeroBit);
        setFlag(NEGATIVE, negBit);
    }

    void readImmediate(unsigned short& PC, char& data, unsigned short address){
        PC = PC + 1;
        address = PC;
        data = readAddress(PC);
    }

    void readZeroPage(unsigned short& PC, char& data, unsigned short address){
        PC = PC + 1;
        address = readAddress(PC);
        data = readAddress(address);
    }

    void readAbsolute(unsigned short& PC, char& data, unsigned short &address){
        PC = PC + 1;
        address = readLittleEndian(PC);
        PC = PC + 1;
        data = readAddress(address);
    }

    /* ZERO PAGE, X: X provided since in two instructions the X changes to Y */
    void readZeroPageX(unsigned short& PC, char& data, unsigned short& address, char X){
        PC = PC + 1;
        address = PC + X;
        data = readAddress(PC+X);
    }

    void readAbsoluteX(unsigned short &PC, char& data, unsigned short & address,char X){
        PC = PC + 1;
        address = readLittleEndian(PC) + X;
        PC = PC + 1;
        data = readAddress(address);
    }
    void processInstruction(char instruction){
        
        char data = 0;
        unsigned short address;
        //check specific instructions here then pattern matching
        if(instruction&0x01){
            /* Check addressing modes */
            switch(instruction&0x1C){
                case 0x0: {
                    PC = PC + 1;
                    address = readLittleEndian(PC+X);
                    PC = PC + 1;
                    data = readAddress(address);
                    break;
                } 
                case 0x1: {
                    readZeroPage(PC, data, address);
                    break;
                }
                case 0x2:{
                    readImmediate(PC, data, address);
                    break;
                }
                case 0x3: {
                    readAbsolute(PC, data, address);
                    break;
                }
                case 0x4: {
                    PC = PC + 1;
                    short address = readLittleEndian(PC)+Y;
                    PC = PC + 1;
                    data = readAddress(address);
                    break;
                }
                case 0x5: {
                    readZeroPageX(PC, data, address, X);
                }
                case 0x6: { 
                    readAbsoluteX(PC, data, address, X);
                    break;
                }
                case 0x7: {
                    readAbsoluteX(PC, data, address, Y); /*read absolute Y */
                    break;
                }
            }

            switch(instruction&0xE0){
                case 0x0: ORA(data); break;
                case 0x1: AND(data); break;
                case 0x2: EOR(data); break; 
                case 0x3: ADC(data); break;
                case 0x4: STA(address); break;  //removed PC = PC -1
                case 0x5: LDA(data); break;
                case 0x6: CMP(data); break;
                case 0x7: SBC(data); break;
            }
        }
        else if(instruction&0x02){
            bool accumulator = false;
            switch(instruction&0x1C){ // check this mask
                case 0x0: {
                    readImmediate(PC, data, address);
                    break;
                }
                case 0x1: {
                    readZeroPage(PC, data, address);
                    break;
                }
                case 0x2: {
                    accumulator = true;
                    //Accumulator
                    break;
                }
                case 0x3: {
                    readAbsolute(PC, data, address);
                    break;
                } 
                case 0x5: {
                    if(instruction&0xE0==0x4||instruction&0xE0==0x5){   //check mask
                        readZeroPageX(PC, data, address, Y);
                    }
                    else{
                        readZeroPageX(PC, data, address, X);
                    }
                    break;
                }
                case 0x7: {
                    if(instruction&0xE0==0x5){
                        readAbsoluteX(PC, data, address, Y);
                    }
                    else{
                        readAbsoluteX(PC, data, address, X);
                    }
                    break;
                }
            }
            switch(instruction&0xE0){
                case 0x0: ASL(address, accumulator); break;
                case 0x1: ROL(address, accumulator); break;
                case 0x2: LSR(address, accumulator); break; 
                case 0x3: ROR(address, accumulator); break;
                case 0x4: STX(address, accumulator); break;
                case 0x5: LDX(data); break;
                case 0x6: DEC(address, accumulator); break;
                case 0x7: INC(address, accumulator); break;
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
        checkValueFlags(A);
    }

    void AND(char data){
        A&=data;
        checkValueFlags(A);
    }
    void EOR(char data){
        A^=data;
        checkValueFlags(A);
    }
    void ADC(char data){
        bool carryBit = A + data > 0xFF ? 1 : 0;
        bool overFlowBit = (A + data > 0x7F || A + data < 0x80);
        setFlag(CARRY, carryBit);
        A+=data;
        checkValueFlags(A);
        setFlag(OVERFLOW, overFlowBit);
    }
    void SBC(char data){
        bool overFlowBit = (A - data > 0x7F || A - data < 0x80);
        bool borrowBit = A >= data ? 1: 0;
        A-=data;
        checkValueFlags(A);
        setFlag(OVERFLOW, overFlowBit);
        setFlag(CARRY, borrowBit);
    }
    void STA(short int address){
        memory[address] = A; // check this
    }
    void LDA(char data){
        A = data; //check if flag is to be set here
    }
    void CMP(char data){
        bool borrowBit = A >= data ? 1: 0;
        setFlag(CARRY, borrowBit);
        checkValueFlags(A - data);
        // Update flags based on CMP
    }

    void ASL(unsigned short address, bool accumulator){

    }
    void ROL(unsigned short address, bool accumulator){
        
    }
    void LSR(unsigned short address, bool accumulator){
        
    }
    void ROR(unsigned short address, bool accumulator){
        
    }
    void STX(unsigned short address, bool accumulator){
        
    }

    void LDX(char data){
        X = data; //check if flag to be set here
    }
    void DEC(unsigned short address, bool accumulator){
        
    }
    void INC(unsigned short address, bool accumulator){
        
    }
};

int main(int argc, char* argv[]){
    NES nes;
    nes.readFile(argv[1]);
}