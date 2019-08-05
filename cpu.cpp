
using namespace std;
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
/*We should handle addresses automaitcally or not? */
/*Look at best practices and refactor */
//Check write addresses
//Introduce cycle accuracy later
//Implement the B flag
//Delay through Cycles
#include "nes.h"
    
NES::NES(char* path){
    PC = 0x8000;
    P = 0x34;
    SP = 0xFF;
    cartridge = new Cartridge(path);
    IRQ = true;
    NMI = true;
    previousNMILevel = true;
}

unsigned char NES::readAddress(unsigned short address){
    if(address<=0x1FFF){
        return memory[address%(0x0800)];
    }
    else if(address==0x2002){
        return 0x80;
    }

    else if(address>=0x2000&&address<=0x3FFF){
        cout << "PPU access" <<endl;
    }

    else if(address==0x4016|| address == 0x4017){
        cout << "INPUT" << endl;
    }
    else if(address>=0x8000&&address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        return cartridge->read(prgRomAddress);
    }
}

void NES::writeAddress(unsigned short address, char value){
    if(address <= 0x1FFF){
        memory[address%0x800] = value;
    }
    else if(address>=0x8000&& address<=0xFFFF){
        short prgRomAddress = address - 0x8000;
        // cartridge->write(prgRomAddress, value);
    }

}

short NES::readLittleEndian(unsigned short address){
    short data = 0;
    data = (unsigned char)readAddress(address+1);
    data = (data)<<8;
    data = data | (unsigned char)readAddress(address); // sign bit extended here
    return data;
}

void NES::setFlag(char mask, bool bit){
    P&=!mask;
    if(bit){
        P|=mask;
    }
}

bool NES::getFlag(char mask){
    return ((P&mask) != 0);
}

void NES::checkValueFlags(char value){
    bool negBit, zeroBit;
    negBit = value < 0 ? 1: 0;
    zeroBit = value == 0? 0: 1;
    setFlag(ZERO, zeroBit);
    setFlag(NEGATIVE, negBit);
}

void NES::printStatus(){
    printf("Program Counter: %x\n", PC);
    printf("Stack pointer: %d", SP);
    printf("Registers: A: %d, X: %d, Y: %d\n", A, X, Y);
    printf("Flags: \n N: %d \n Z: %d \n  Carry: %d \n Overflow: %d \n Interrupt Disable: %d\n", \
    getFlag(NEGATIVE),\
    getFlag(ZERO), \
    getFlag(CARRY), \
    getFlag(OVERFLOW), \
    getFlag(INT) \
    );
    printf("IRQ: %d, NMI: %d\n\n", IRQ, NMI);
}

void NES::readImmediate(unsigned short& PC, char& data, unsigned short address){
    cout << "IMMEDIATE ";
    PC = PC + 1;
    address = PC;
    data = readAddress(PC);
}

void NES::readZeroPage(unsigned short& PC, char& data, unsigned short address){
    cout << "ZERO PAGE ";
    PC = PC + 1;
    address = ((0x00)<<8)|readAddress(PC);
    data = readAddress(address);
}

void NES::readAbsolute(unsigned short& PC, char& data, unsigned short &address){
    cout << "ABSOLUTE ";
    PC = PC + 1;
    address = readLittleEndian(PC);
    PC = PC + 1;
    data = readAddress(address);
}

/* ZERO PAGE, X: X provided since in two instructions the X changes to Y */
void NES::readZeroPageX(unsigned short& PC, char& data, unsigned short& address, char X){
    cout << "ZEROPAGE X" ;
    PC = PC + 1;
    address = PC + X;
    data = readAddress(PC+X);
}

void NES::readAbsoluteX(unsigned short &PC, char& data, unsigned short & address,char X){
    cout <<"ABSOLUTE X ";
    PC = PC + 1;
    address = readLittleEndian(PC) + X;
    PC = PC + 1;
    data = readAddress(address);
}
void NES::processInstruction(unsigned char instruction){
    
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
        data = readAddress(PC);
        bool bit = (instruction & 0x20) >> 5;
        switch((instruction&0xC0)>>6){
            case 0x0: BRANCH(NEGATIVE, bit, data); break;
            case 0x1: BRANCH(OVERFLOW, bit, data); break;
            case 0x2: BRANCH(CARRY, bit, data); break;
            case 0x3: BRANCH(ZERO, bit, data); break;
        }
    }
    else if(cc==0x01){
        /* Check addressing modes */
        switch(bbb){
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
                cout << "(ABSOLUTE, Y)" << endl;
                PC = PC + 1;
                short address = readLittleEndian(PC)+Y;
                PC = PC + 1;
                data = readAddress(address);
                break;
            }
            case 0x5: {
                readZeroPageX(PC, data, address, X);
                break;
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
                case 0x2: JMP(address); break; //JMP
                case 0x3: JMP_ABS(address); break; //JMP ABS
                case 0x4: STY(address); break; //STY
                case 0x5: LDY(data); break; //LDY
                case 0x6: CPY(data); break; //CPY
                case 0x7: CPX(data); break; //CPX 
            }
        }

    }
    
}
void NES::cycle(){
    short address;
    if(!NMI&&previousNMILevel){
        previousNMILevel = true;
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = readLittleEndian(0xFFFA);
        PC = address;
    }
    else if(!IRQ&&!getFlag(INT)){
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = readLittleEndian(0xFFFE);
        PC = address; 
    }
    char instruction = readAddress(PC);
    processInstruction(instruction);
    PC = PC + 1; //check for jump
}
void NES::ORA(char data){
    cout << "ORA" << endl;
    A|=data;
    checkValueFlags(A);
}

void NES::AND(char data){
    cout << "AND" << endl;
    A&=data;
    checkValueFlags(A);
}
void NES::EOR(char data){
    cout << "EOR" << endl;
    A^=data;
    checkValueFlags(A);
}
void NES::ADC(char data){
    cout<< "ADC" << endl;
    bool carryBit = (A + data) > 0xFF ? 1 : 0;
    bool overFlowBit = (A + data > 0x7F || A + data < 0x80);
    setFlag(CARRY, carryBit);
    A+=data;
    checkValueFlags(A);
    setFlag(OVERFLOW, overFlowBit); //fix this and have carry
}
void NES::SBC(char data){
    cout << "SBC" <<endl;
    bool overFlowBit = (A - data > 0x7F || A - data < 0x80);
    bool borrowBit = A >= data ? 1: 0;
    A-=data;
    checkValueFlags(A);
    setFlag(OVERFLOW, overFlowBit);
    setFlag(CARRY, borrowBit);
}
void NES::STA(short int address){
    cout << "STA" <<endl;
    writeAddress(address, A);  // check this
}
void NES::LDA(char data){
    cout << "LDA" <<endl;
    A = data; //check if flag is to be set here
    checkValueFlags(A);
}
void NES::CMP(char data){
    cout << "CMP" << endl;
    bool borrowBit = A >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(A - data);
    // Update flags based on CMP
}

void NES::ASL(char data, unsigned short address, bool accumulator){
    bool carryBit = 0;
    if(accumulator){
        carryBit = (A & 0x80) >> 7;
        A = A << 1;
        checkValueFlags(A);
        
    }
    else{
        carryBit = (data & 0x80) >> 7;
        writeAddress(address, data << 1);
        checkValueFlags(data);
    }
    setFlag(CARRY, carryBit);
    cout << "ASL" <<endl;
}
void NES::ROL(char data, unsigned short address, bool accumulator){
    bool zeroBit = getFlag(CARRY);
    bool carryBit = 0;
    if(accumulator){
        carryBit = (A & 0x80) >> 7;
        A  = A << 1;
        A &= 0xFE;
        A |= zeroBit ? 0x01: 0;
        checkValueFlags(A);
    }
    else{
        carryBit = (data & 0x8F) >> 7;
        data = data << 1;
        data&= 0xFE;
        data|= zeroBit ? 0x1: 0;
        writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, carryBit);
    cout << "ROL" << endl;
}
void NES::LSR(char data, unsigned short address, bool accumulator){
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
void NES::ROR(char data, unsigned short address, bool accumulator){
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
void NES::STX(char data, unsigned short address, bool accumulator){
    writeAddress(address, X);
    cout << "STX" << endl; //check if flag is set here
}

void NES::LDX(char data){
    cout<< "LDX" <<endl;
    X = data; //check if flag to be set here
    checkValueFlags(X);
}
void NES::DEC(char data, unsigned short address, bool accumulator){
    writeAddress(address, data - 1);
    checkValueFlags(data -1);
    cout << "DEC" <<endl;
}
void NES::INC(char data, unsigned short address, bool accumulator){
    writeAddress(address, data + 1);
    checkValueFlags(data + 1);
    cout << "INC" << endl;
}

void NES::BIT(char data){
    // Check for immediate instruction
    cout << "BIT" << endl;
    bool zeroBit, overflowBit, negativeBit;
    zeroBit = data & A > 0 ? 1 : 0;
    negativeBit = data & 0x80;
    overflowBit = data & 0x70;
    setFlag(ZERO, data & A);
    setFlag(OVERFLOW, overflowBit);
    setFlag(NEGATIVE, negativeBit);
}

void NES::JMP(unsigned short address){
    cout << "JMP" << endl;
    if(!address&0xFF){
        PC = readLittleEndian(address);
    }
    else{
        short jumpTo = (readAddress(address&0xFF<<8) << 8) | (readAddress(address));
        PC = jumpTo;
    }
    
}

void NES::JMP_ABS(unsigned short address){
    cout << "JMP_ABS" << endl;
    PC = address;

    //check for page boundaries
}

void NES::STY(unsigned short address){
    cout << "STY" << endl;
    writeAddress(address, Y);
}

void NES::LDY(char data){
    cout << "LDY" << endl;
    Y = data;
    checkValueFlags(data);
}

void NES::CPY(char data){
    cout << "CPY" << endl;
    bool borrowBit = Y >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(Y - data);

}
void NES::CPX(char data){
    cout << "CPX" << endl;
    bool borrowBit = X >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(X - data);
}

void NES::BRANCH(masks flag, bool bit, char data){
    cout << "BRANCH" << endl;
    if(getFlag(flag)==bit){
        PC = PC + data - 2; //chedck this
    }
}
void NES::push(char data){
    char highByte = 0x1F;
    writeAddress(highByte << 8 | SP, data);
    SP = SP - 1;
}

void NES::pushLittleEndian(short data){
    char highByte = (data>>8);
    char lowByte = (data&0x0F);
    push(highByte);
    push(lowByte);
}

char NES::pop(){
    char highByte = 0x1F;
    SP = SP + 1;
    char data = readAddress(highByte << 8 | SP);
    return data;
}

short NES::popLittleEndian(){
    short result = pop();
    result = (pop() << 8) | result;
    return result;
}

void NES::BRK(){
    cout << "BRK" << endl;
    setFlag(INT, 1);
    IRQ = false;
    pushLittleEndian(PC+2);
    push(P);
}

void NES::JSR(){
    cout << "JSR" << endl;
    pushLittleEndian(PC+2);
    PC = PC + 1;
    short address = readLittleEndian(PC);
    address = address - 1;
    PC = address;
    //check correct PC behavior
}

void NES::RTI(){
    cout << "RTI" << endl;
    P = pop();
    PC = popLittleEndian();
    IRQ = true;
    NMI = true;
    previousNMILevel = true; //check this
}

void NES::RTS(){
    cout << "RTS" << endl;
    PC = popLittleEndian();
    PC = PC + 1; //check this
}    

void NES::PHP(){
    cout << "PHP" << endl;
    push(P);
}

void NES::PLP(){
    cout << "PLP" << endl;
    P = pop();
}

void NES::PHA(){
    cout << "PHA" << endl;
    push(A);
}

void NES::PLA(){
    cout << "PLA" << endl;
    A = pop();
}

void NES::DEY(){
    cout << "DEY" << endl;
    Y = Y - 1;
    checkValueFlags(Y);
}

void NES::TAY(){
    cout << "TAY" << endl;
    Y = A;
    checkValueFlags(Y);
}

void NES::INY(){
    cout << "INY" << endl;
    Y = Y + 1;
    checkValueFlags(Y);
}

void NES::INX(){
    cout << "INX" << endl;
    X = X + 1;
    checkValueFlags(X);
}

void NES::CLC(){
    cout << "CLC" << endl;
    setFlag(CARRY, 0);
}
void NES::SEC(){
    cout << "SEC" << endl;
    setFlag(CARRY, 1);
}
void NES::CLI(){
    cout << "CLI" << endl;
    setFlag(INT, 0);
}

void NES::SEI(){
    cout << "SEI" << endl;
    setFlag(INT, 1);
}

void NES::TYA(){
    cout << "TYA" << endl;
    A = Y;
    checkValueFlags(A);
}   

void NES::CLV(){
    cout << "CLV" << endl;
    setFlag(OVERFLOW, 0);
}

void NES::CLD(){
    cout << "CLD" << endl;
}


void NES::SED(){
    cout << "SED" << endl;
}

void NES::TXA(){
    cout << "TXA" << endl;
    A = X;
    checkValueFlags(A);
}

void NES::TXS(){
    cout << "TXS" << endl;
    SP = X;
}

void NES::TAX(){
    cout << "TAX" << endl;
    X = A;
}

void NES::TSX(){
    cout << "TSX" << endl;
    X = SP;
}

void NES::DEX(){
    cout << "DEX" << endl;
    X = X - 1;
}

void NES::NOP(){
    cout << "NOP" << endl; 
}


//What happens on reset and what happens every cycle
//Check JUMP on edge or what?
//Check reset
//Do CPU and PPU cycle together? Can the CPU and the PPU both do something in the same cycle