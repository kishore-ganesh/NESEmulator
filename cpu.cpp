#include<stdio.h>
#include<iostream>
using namespace std;
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
/*We should handle addresses automaitcally or not? */
//Check write addresses
#include "cartridge.h"
class NES{
    char A, X, Y, P, SP; //check check setting of stack pointer
    Cartridge* cartridge;
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
    
    unsigned char display [256][240]; //take care of x and y
    public: 
    NES(char* path){
        PC = 0x8000;
        P = 0x34;
        cartridge = new Cartridge(path);
    }
    char readAddress(unsigned short address){
        if(address<=0x1FFF){
            return memory[address%(0x0800)];
        }
        else if(address>=0x8000&&address<=0xFFFF){
            short prgRomAddress = address - 0x8000;
            return cartridge->read(prgRomAddress);
        }
    }

    void writeAddress(unsigned short address, char value){
        if(address <= 0x1FFF){
            memory[address%0x800] = value;
        }
        else if(address>=0x8000&& address<=0xFFFF){
            short prgRomAddress = address - 0x8000;
            cartridge->write(prgRomAddress, value);
        }

    }
    short readLittleEndian(unsigned short address){
        short data = 0;
        data = readAddress(address+1);
        data = (data)<<4;
        data = data | readAddress(address);
    }

    void setFlag(char mask, bool bit){
        P&=!mask;
        if(bit){
            P|=mask;
        }
    }

    bool getFlag(char mask){
        return P&mask > 0;
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
        if(instruction&0x1F==0x10){
            PC = PC + 1;
            data = readAddress(address);
            bool bit = instruction & 0x20;
            switch(instruction&0xC0){
                case 0x0: BRANCH(NEGATIVE, bit, data); break;
                case 0x1: BRANCH(OVERFLOW, bit, data); break;
                case 0x2: BRANCH(CARRY, bit, data); break;
                case 0x3: BRANCH(ZERO, bit, data); break;
            }
        }
        else if(instruction&0x01){
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
        else if(instruction&0x02 || instruction&0x03==0){
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
            if(instruction & 0x02){
                switch(instruction&0xE0){
                    case 0x0: ASL(data, address, accumulator); break;
                    case 0x1: ROL(data, address, accumulator); break;
                    case 0x2: LSR(data, address, accumulator); break; 
                    case 0x3: ROR(data, address, accumulator); break;
                    case 0x4: STX(data, address, accumulator); break;
                    case 0x5: LDX(data); break;
                    case 0x6: DEC(data, address, accumulator); break;
                    case 0x7: INC(data, address, accumulator); break;
                    }                
            }
            else{
                switch(instruction & 0xE0){
                    case 0x1: break; //BIT 
                    case 0x2: break; //JMP
                    case 0x3: break; //JMP ABS
                    case 0x4: break; //STY
                    case 0x5: break; //LDY
                    case 0x6: break; //CPY
                    case 0x7: break; //CPX 
                }
            }

        }
        
    }
    void cycle(){
        char instruction = readAddress(PC);
        processInstruction(instruction);
        PC = PC + 1; //check for jump
    }
    void ORA(char data){
        cout << "ORA" << endl;
        A|=data;
        checkValueFlags(A);
    }

    void AND(char data){
        cout << "AND" << endl;
        A&=data;
        checkValueFlags(A);
    }
    void EOR(char data){
        cout << "EOR" << endl;
        A^=data;
        checkValueFlags(A);
    }
    void ADC(char data){
        cout<< "ADC" << endl;
        bool carryBit = A + data > 0xFF ? 1 : 0;
        bool overFlowBit = (A + data > 0x7F || A + data < 0x80);
        setFlag(CARRY, carryBit);
        A+=data;
        checkValueFlags(A);
        setFlag(OVERFLOW, overFlowBit);
    }
    void SBC(char data){
        cout << "SBC" <<endl;
        bool overFlowBit = (A - data > 0x7F || A - data < 0x80);
        bool borrowBit = A >= data ? 1: 0;
        A-=data;
        checkValueFlags(A);
        setFlag(OVERFLOW, overFlowBit);
        setFlag(CARRY, borrowBit);
    }
    void STA(short int address){
        cout << "STA" <<endl;
        memory[address] = A; // check this
    }
    void LDA(char data){
        cout << "LDA" <<endl;
        A = data; //check if flag is to be set here
        checkValueFlags(A);
    }
    void CMP(char data){
        cout << "CMP" << endl;
        bool borrowBit = A >= data ? 1: 0;
        setFlag(CARRY, borrowBit);
        checkValueFlags(A - data);
        // Update flags based on CMP
    }

    void ASL(char data, unsigned short address, bool accumulator){
        bool carryBit = 0;
        if(accumulator){
            carryBit = A & 0x8F;
            A = A << 1;
            checkValueFlags(A);
            
        }
        else{
            carryBit = data & 0x8F;
            writeAddress(address, data << 1);
            checkValueFlags(data);
        }
        setFlag(CARRY, carryBit);
        cout << "ASL" <<endl;
    }
    void ROL(char data, unsigned short address, bool accumulator){
        bool zeroBit = getFlag(CARRY);
        bool carryBit = 0;
        if(accumulator){
            carryBit = A & 0x8F;
            A  = A << 1;
            A &= 0xFE;
            A |= zeroBit ? 0x01: 0;
            checkValueFlags(A);
        }
        else{
            carryBit = data & 0x8F;
            data = data << 1;
            data&= 0xFE;
            data|= zeroBit ? 0x1: 0;
            writeAddress(address, data);
            checkValueFlags(data);
        }
        setFlag(CARRY, carryBit);
        cout << "ROL" << endl;
    }
    void LSR(char data, unsigned short address, bool accumulator){
        bool nextCarryBit = 0;
        if(accumulator){
            nextCarryBit = A & 0x01;
            A = A >> 1;
            checkValueFlags(A);
        }
        else{
            nextCarryBit = data & 0x01;
            data = data >> 1;
            writeAddress(address, data);
            checkValueFlags(data);
        }
        setFlag(CARRY, nextCarryBit);
        cout << "LSR" << endl;
    }
    void ROR(char data, unsigned short address, bool accumulator){
        bool previousCarryBit = getFlag(CARRY);
        bool nextCarryBit = 0;
        if(accumulator){
            nextCarryBit = A & 0x01;
            A = A >> 1;
            A &= 0x7F;
            A |= previousCarryBit ? 0x80 : 0;
            checkValueFlags(A);
        }
        else{
            nextCarryBit = data & 0x01;
            data = data >> 1;
            data &= 0x7F;
            data |= previousCarryBit ? 0x80 : 0;
            writeAddress(address, data);
            checkValueFlags(data);
        }
        setFlag(CARRY, nextCarryBit);
        cout << "ROR" << endl;
    }
    void STX(char data, unsigned short address, bool accumulator){
        writeAddress(address, X);
        cout << "STX" << endl; //check if flag is set here
    }

    void LDX(char data){
        cout<< "LDX" <<endl;
        X = data; //check if flag to be set here
        checkValueFlags(X);
    }
    void DEC(char data, unsigned short address, bool accumulator){
        writeAddress(address, data - 1);
        checkValueFlags(data -1);
        cout << "DEC" <<endl;
    }
    void INC(char data, unsigned short address, bool accumulator){
        writeAddress(address, data + 1);
        checkValueFlags(data + 1);
        cout << "INC" << endl;
    }

    void BIT(char data){
        // Check for immediate instruction
        bool zeroBit, overflowBit, negativeBit;
        zeroBit = data & A > 0 ? 1 : 0;
        negativeBit = data & 0x80;
        overflowBit = data & 0x70;
        setFlag(ZERO, data & A);
        setFlag(OVERFLOW, overflowBit);
        setFlag(NEGATIVE, negativeBit);
    }

    void JMP(unsigned short address){
        if(!address&0xFF){
            PC = readLittleEndian(address);
        }
        else{
            short jumpTo = (readAddress(address&0xFF<<8) << 8) | (readAddress(address));
            PC = jumpTo;
        }
        
    }

    void JMP_ABS(unsigned short address){
        PC = address;

        //check for page boundaries
    }

    void STY(unsigned short address){
        writeAddress(address, Y);
    }
    
    void LDY(char data){
        Y = data;
        checkValueFlags(data);
    }

    void CPY(char data){
        bool borrowBit = Y >= data ? 1: 0;
        setFlag(CARRY, borrowBit);
        checkValueFlags(Y - data);

    }
    void CPX(char data){
        bool borrowBit = X >= data ? 1: 0;
        setFlag(CARRY, borrowBit);
        checkValueFlags(X - data);
    }

    void BRANCH(masks flag, bool bit, char data){
        if(getFlag(flag)==bit){
            PC = PC + data - 1 ; //chedck this
        }
    }

};

int main(int argc, char* argv[]){
    NES nes(argv[1]);
    while(true){
        nes.cycle();
    }
}