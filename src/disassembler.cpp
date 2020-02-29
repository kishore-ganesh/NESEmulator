#include "memory.h"
void processInstruction(unsigned char instruction, Memory* memory){
    char data = 0;
    unsigned short address;
    char aaa = (instruction & 0xE0)>>5;
    char bbb = (instruction & 0x1C) >> 2;
    char cc = (instruction & 0x03);
    /*
    Several instructions have patterns
        */
    //check specific instructions here then pattern matching
    switch(instruction){
        case 0x00: BRK(); return;
        case 0x20: JSR(); return;
        case 0x40: RTI(); return;
        case 0x60: RTS(); return;
        case 0x08: PHP(); return;
        case 0x28: PLP(); return;
        case 0x48: PHA(); return;
        case 0x68: PLA(); return;
        case 0x88: DEY(); return;
        case 0xA8: TAY(); return;
        case 0xC8: INY(); return;
        case 0xE8: INX(); return;
        case 0x18: CLC(); return;
        case 0x38: SEC(); return;
        case 0x58: CLI(); return;
        case 0x78: SEI(); return;
        case 0x98: TYA(); return;
        case 0xB8: CLV(); return;
        case 0xD8: CLD(); return;
        case 0xF8: SED(); return;
        case 0x8A: TXA(); return;
        case 0x9A: TXS(); return;
        case 0xAA: TAX(); return;
        case 0xBA: TSX(); return;
        case 0xCA: DEX(); return;
        case 0xEA: NOP(); return; 

    }
    if((instruction&0x1F)==0x10){
        PC = PC + 1;
        data = memory->readAddress(PC);
        bool bit = (instruction & 0x20) >> 5;
        switch((instruction&0xC0)>>6){
            case 0x0: printf("Branch to %d if bit is %d": BRANCH(NEGATIVE, bit, data); break;
            case 0x1: BRANCH(INTEGER_OVERFLOW, bit, data); break;
            case 0x2: BRANCH(CARRY, bit, data); break;
            case 0x3: BRANCH(ZERO, bit, data); break;
        }
    }
    else if(cc==0x01){
        /* Check addressing modes */
        switch(bbb){
            case 0x0: {
                cout << "(Indirect, X)" << endl;
                PC = PC + 1;
                address = memory->readAddress(PC);
                address = memory->readLittleEndian((address+X)&0x00FF);
                data = memory->readAddress(address);
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
                cout << "(Indirect, Y)" << endl;
                PC = PC + 1;
                address = (memory->readAddress(PC)) & 0x00FF;
                address = memory->readLittleEndian(address) + Y;
                data = memory->readAddress(address);                
                break;
            }
            case 0x5: {
                readZeroPageX(PC, data, address, X);
                break;
            }
            case 0x6: { 
                readAbsoluteX(PC, data, address, Y);
                break;
            }
            case 0x7: {
                readAbsoluteX(PC, data, address, X); /*read absolute Y */
                break;
            }
        }

        switch(aaa){
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
    else if(cc==0x02 || cc==0x00){
        bool accumulator = false;
        switch(bbb){ // check this mask
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
                if(aaa==0x4||aaa==0x5){   //check mask
                    readZeroPageX(PC, data, address, Y);
                }
                else{
                    readZeroPageX(PC, data, address, X);
                }
                break;
            }
            case 0x7: {
                if(aaa==0x5){
                    readAbsoluteX(PC, data, address, Y);
                }
                else{
                    readAbsoluteX(PC, data, address, X);
                }
                break;
            }
        }
        if(cc==2){
            switch(aaa){
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
            switch(aaa){
                case 0x1: BIT(data); break;  //BIT 
                case 0x2: JMP_ABS(address); break; //JMP ABS
                case 0x3: JMP(address); break; //JMP
                case 0x4: STY(address); break; //STY
                case 0x5: LDY(data); break; //LDY
                case 0x6: CPY(data); break; //CPY
                case 0x7: CPX(data); break; //CPX 
            }
        }

    }

}
int main(int argc, char* argv[]){
    Memory* memory = new Memory(argv[1]);
    short PC = memory->readLittleEndian(0xFFFC);
    while(true){
        unsigned char instruction = memory->readAddress(PC);
        processInstruction(PC);
    }
}