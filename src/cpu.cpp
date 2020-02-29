#include "cpu.h"
using std::cout;
using std::endl;
CPU::CPU(Memory* memory){
    this->memory = memory;
    PC = memory->readLittleEndian(0xFFFC);
    P = 0x34;
    SP = 0xFF;
    NMI.clearInterrupt(); //Make NMI a pointer
    IRQ = true;
}

EdgeInterrupt* CPU::getNMIPointer(){
    return &NMI;
}
void CPU::setFlag(char mask, bool bit){
    P&=~mask;
    if(bit){
        P|=mask;
    }
}

bool CPU::getFlag(char mask){
    return ((P&mask) != 0);
}

void CPU::checkValueFlags(char value){
    bool negBit, zeroBit;
    negBit = value < 0 ? 1: 0;
    zeroBit = value == 0? 1: 0;
    setFlag(ZERO, zeroBit);
    setFlag(NEGATIVE, negBit);
}

void CPU::printStatus(){
    spdlog::info("Program Counter: {0:x}", PC);
    spdlog::info("Stack pointer: {0:d}", SP);
    spdlog::info("Registers: A: {0:d}, X: {1:d}, Y: {2:d}", A, X, Y);
    spdlog::info("Flags: \n N: {0:d} \n Z: {1:d} \n  Carry: {2:d} \n Overflow: {3:d} \n Interrupt Disable: {4:d}\n", \
    getFlag(NEGATIVE),\
    getFlag(ZERO), \
    getFlag(CARRY), \
    getFlag(INTEGER_OVERFLOW), \
    getFlag(INT) \
    );
    
}

unsigned char CPU::readAddress(unsigned short address){
    cycles++;
    return memory->readAddress(address);
}
short CPU::readLittleEndian(unsigned short address){
    cycles+=2;
    return memory->readLittleEndian(address);
}
void CPU::writeAddress(unsigned short address, char value){
    cycles++;
    memory->writeAddress(address, value);
}

void CPU::readImmediate(unsigned short& PC, unsigned short& address){
    spdlog::info("IMMEDIATE ");
    PC = PC + 1;
    address = PC;
}

void CPU::readZeroPage(unsigned short& PC, unsigned short& address){
    spdlog::info("ZERO PAGE ");
    PC = PC + 1;
    address = (readAddress(PC))&0x00FF;
}

void CPU::readAbsolute(unsigned short& PC, unsigned short &address){
    spdlog::info("ABSOLUTE ");
    PC = PC + 1;
    address = readLittleEndian(PC);
    PC = PC + 1;
}

/* ZERO PAGE, X: X provided since in two instructions the X changes to Y */
void CPU::readZeroPageX(unsigned short& PC, unsigned short& address, unsigned char X){
    spdlog::info("ZEROPAGE X");
    PC = PC + 1;
    address = (readAddress(PC)+ X)&0x00FF;
    // address = readAddress((0x00FF)&address);
}

void CPU::readAbsoluteX(unsigned short &PC, unsigned short & address,unsigned char X){
    spdlog::info("ABSOLUTE X ");
    PC = PC + 1;
    address = readLittleEndian(PC) + X;
    PC = PC + 1;
}
void CPU::processInstruction(unsigned char instruction){
    
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
        bool bit = (instruction & 0x20) >> 5;
        switch((instruction&0xC0)>>6){
            case 0x0: BRANCH(NEGATIVE, bit); break;
            case 0x1: BRANCH(INTEGER_OVERFLOW, bit); break;
            case 0x2: BRANCH(CARRY, bit); break;
            case 0x3: BRANCH(ZERO, bit); break;
        }
    }
    else if(cc==0x01){
        /* Check addressing modes */
        switch(bbb){
            case 0x0: {
                spdlog::info("(Indirect, X)") ;
                PC = PC + 1;
                address = readAddress(PC);
                address = readLittleEndian((address+X)&0x00FF);
                break;
            } 
            case 0x1: {
                readZeroPage(PC, address);
                break;
            }
            case 0x2:{
                readImmediate(PC, address);
                break;
            }
            case 0x3: {
                readAbsolute(PC, address);
                break;
            }
            case 0x4: {
                spdlog::info("(Indirect, Y)") ;
                PC = PC + 1;
                address = (readAddress(PC)) & 0x00FF;
                address = readLittleEndian(address) + Y;                
                break;
            }
            case 0x5: {
                readZeroPageX(PC, address, X);
                break;
            }
            case 0x6: { 
                readAbsoluteX(PC, address, Y);
                break;
            }
            case 0x7: {
                readAbsoluteX(PC, address, X); /*read absolute Y */
                break;
            }
        }

        switch(aaa){
            case 0x0: ORA(address); break;
            case 0x1: AND(address); break;
            case 0x2: EOR(address); break; 
            case 0x3: ADC(address); break;
            case 0x4: STA(address); break;  //removed PC = PC -1
            case 0x5: LDA(address); break;
            case 0x6: CMP(address); break;
            case 0x7: SBC(address); break;
        }
    }
    else if(cc==0x02 || cc==0x00){
        bool accumulator = false;
        switch(bbb){ // check this mask
            case 0x0: {
                readImmediate(PC, address);
                break;
            }
            case 0x1: {
                readZeroPage(PC, address);
                break;
            }
            case 0x2: {
                accumulator = true;
                //Accumulator
                break;
            }
            case 0x3: {
                readAbsolute(PC, address);
                break;
            } 
            case 0x5: {
                if((aaa==0x4||aaa==0x5)&&cc==0x02){   //check mask
                    readZeroPageX(PC, address, Y);
                }
                else{
                    readZeroPageX(PC, address, X);
                }
                break;
            }
            case 0x7: {
                if(aaa==0x5&&cc==0x02){
                    readAbsoluteX(PC, address, Y);
                }
                else{
                    readAbsoluteX(PC, address, X);
                }
                break;
            }
        }
        if(cc==2){
            switch(aaa){
                case 0x0: ASL(address, accumulator); break;
                case 0x1: ROL(address, accumulator); break;
                case 0x2: LSR(address, accumulator); break; 
                case 0x3: ROR(address, accumulator); break;
                case 0x4: STX(address, accumulator); break;
                case 0x5: LDX(address); break;
                case 0x6: DEC(address, accumulator); break;
                case 0x7: INC(address, accumulator); break;
                }                
        }
        else{
            switch(aaa){
                case 0x1: BIT(address); break;  //BIT 
                case 0x2: JMP_ABS(address); break; //JMP ABS
                case 0x3: JMP(address); break; //JMP
                case 0x4: STY(address); break; //STY
                case 0x5: LDY(address); break; //LDY
                case 0x6: CPY(address); break; //CPY
                case 0x7: CPX(address); break; //CPX 
            }
        }

    }
    
}
int CPU::cycle(){
    cycles = 0;
    short address;
    if(NMI.checkInterrupt()){
        spdlog::info("NMI Interrupt occured");
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = readLittleEndian(0xFFFA);
        PC = address;
    }
    else if(!IRQ&&!getFlag(INT)){
        spdlog::info("Interrupt occured");
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = readLittleEndian(0xFFFE);
        PC = address; 
    }
    char instruction = readAddress(PC);
    processInstruction(instruction);
    PC = PC + 1; //check for jump
    return cycles;
}
void CPU::ORA(unsigned short address){
    char data = readAddress(address);
    spdlog::info("ORA");
    A|=data;
    checkValueFlags(A);
}

void CPU::AND(unsigned short address){
    char data = readAddress(address);
    spdlog::info("AND");
    A&=data;
    checkValueFlags(A);
}
void CPU::EOR(unsigned short address){
    char data = readAddress(address);
    spdlog::info("EOR");
    A^=data;
    checkValueFlags(A);
}
void CPU::ADC(unsigned short address){
    unsigned char data = readAddress(address);
    spdlog::info("ADC");
    short result = A + data+getFlag(CARRY);
    bool carryBit = result > 0xFF ? 1 : 0;
    bool overFlowBit = (result > 127 || result < -128);
    A = A + data + getFlag(CARRY);
    checkValueFlags(A);
    setFlag(CARRY, carryBit);
    setFlag(INTEGER_OVERFLOW, overFlowBit); //fix this and have carry
}
void CPU::SBC(unsigned short address){
    spdlog::info("SBC");
    unsigned char data = readAddress(address);
    short result = A - data - !getFlag(CARRY);
    bool overFlowBit = (result > 127 || result < -128);
    bool borrowBit = result >= 0 ? 1: 0;
    A = A - data - !getFlag(CARRY);
    checkValueFlags(A);
    setFlag(INTEGER_OVERFLOW, overFlowBit);
    setFlag(CARRY, borrowBit);
}
void CPU::STA(unsigned short address){
    spdlog::info("STA");
    writeAddress(address, A);  // check this
}
void CPU::LDA(unsigned short address){
    unsigned char data = readAddress(address);
    spdlog::info("LDA");
    A = data; //check if flag is to be set here
    checkValueFlags(A);
}
void CPU::CMP(unsigned short address){
    unsigned char data = readAddress(address);
    spdlog::info("CMP");
    bool borrowBit = A >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(A - data);
    // Update flags based on CMP
}

void CPU::ASL(unsigned short address, bool accumulator){
    bool carryBit = 0;
    if(accumulator){
        carryBit = (A & 0x80) >> 7;
        A = A << 1;
        checkValueFlags(A);
        
    }
    else{
        unsigned char data = readAddress(address);
        carryBit = (data & 0x80) >> 7;
        writeAddress(address, data << 1);
        checkValueFlags(data << 1);
    }
    setFlag(CARRY, carryBit);
    spdlog::info("ASL");
}
void CPU::ROL(unsigned short address, bool accumulator){
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
        unsigned char data = readAddress(address);
        carryBit = (data & 0x8F) >> 7;
        data = data << 1;
        data&= 0xFE;
        data|= zeroBit ? 0x1: 0;
        writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, carryBit);
    spdlog::info("ROL");
}
void CPU::LSR(unsigned short address, bool accumulator){
    bool nextCarryBit = 0;
    if(accumulator){
        nextCarryBit = A & 0x01;
        A = (unsigned char)A >> 1;
        checkValueFlags(A);
    }
    else{
        unsigned char data = readAddress(address);
        nextCarryBit = data & 0x01;
        data = (unsigned char)data >> 1;
        writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, nextCarryBit);
    spdlog::info("LSR");
}
void CPU::ROR(unsigned short address, bool accumulator){
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
        unsigned char data = readAddress(address);
        nextCarryBit = data & 0x01;
        data = data >> 1;
        data &= 0x7F;
        data |= previousCarryBit ? 0x80 : 0;
        writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, nextCarryBit);
    spdlog::info("ROR");
}
void CPU::STX(unsigned short address, bool accumulator){
    writeAddress(address, X);
    spdlog::info("STX"); //check if flag is set here
}

void CPU::LDX(unsigned short address){
    unsigned char data = readAddress(address);
    spdlog::info("LDX");
    X = data; //check if flag to be set here
    checkValueFlags(X);
}
void CPU::DEC(unsigned short address, bool accumulator){
    unsigned char data = readAddress(address);
    writeAddress(address, data - 1);
    checkValueFlags(data -1);
    spdlog::info("DEC");
}
void CPU::INC(unsigned short address, bool accumulator){
    unsigned char data = readAddress(address);
    writeAddress(address, data + 1);
    checkValueFlags(data + 1);
    spdlog::info("INC");
}

void CPU::BIT(unsigned short address){
    // Check for immediate instruction
    spdlog::info("BIT");
    unsigned char data = readAddress(address);
    bool zeroBit, overflowBit, negativeBit;
    zeroBit = data & A > 0 ? 1 : 0;
    negativeBit = data & 0x80;
    overflowBit = data & 0x70;
    setFlag(ZERO, data & A);
    setFlag(INTEGER_OVERFLOW, overflowBit);
    setFlag(NEGATIVE, negativeBit);
}

void CPU::JMP(unsigned short address){
    spdlog::info("JMP");
    if(!((address&0x00FF)==0x00FF)){
        PC = readLittleEndian(address) - 1;
    }
    else{
        short jumpTo = (readAddress(address&0xFF00) << 8) | (readAddress(address));
        PC = jumpTo - 1 ;
    }
    
}

void CPU::JMP_ABS(unsigned short address){
    spdlog::info("JMP_ABS");
    PC = address - 1;

    //check for page boundaries
}

void CPU::STY(unsigned short address){
    spdlog::info("STY");
    writeAddress(address, Y);
}

void CPU::LDY(unsigned short address){
    unsigned char data = readAddress(address);
    spdlog::info("LDY");
    Y = data;
    checkValueFlags(data);
}

void CPU::CPY(unsigned short address){
    spdlog::info("CPY");
    unsigned char data = readAddress(address);
    bool borrowBit = Y >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(Y - data);

}
void CPU::CPX(unsigned short address){
    unsigned char data = readAddress(address);
    spdlog::info("CPX");
    bool borrowBit = X >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(X - data);
}

void CPU::BRANCH(masks flag, bool bit){
    PC = PC + 1;
    char data = readAddress(PC);
    spdlog::info("BRANCH");
    if(getFlag(flag)==bit){
        PC = PC + data; //chedck this
    }
}
void CPU::push(char data){
    unsigned char highByte = 0x01;
    short stackAddress = (highByte << 8) | (unsigned char)SP;
    writeAddress(stackAddress, data);
    SP = SP - 1;
    
    //Make it wrap around to prevent overflow
}

void CPU::pushLittleEndian(short data){
    char highByte = (data>>8);
    char lowByte = (data&0x00FF);
    push(highByte);
    push(lowByte);
}

char CPU::pop(){
    unsigned char highByte = 0x01;
    SP = SP + 1;
    short stackAddress = (highByte << 8 )| (unsigned char)SP;
    char data = readAddress(stackAddress);
    return data;
}

short CPU::popLittleEndian(){
    short result = (unsigned char) pop();
    result = (((unsigned char)pop() )<< 8) | result;
    return result;
}

void CPU::BRK(){
    spdlog::info("BRK");
    setFlag(INT, 1);
    IRQ = false;
    pushLittleEndian(PC+2);
    push(P);
    cycles+=1;
}

void CPU::JSR(){
    spdlog::info("JSR");
    pushLittleEndian(PC+2);
    PC = PC + 1;
    short address = readLittleEndian(PC);
    address = address - 1;
    PC = address;
    //check correct PC behavior
}

void CPU::RTI(){
    spdlog::info("RTI");
    P = pop();
    PC = popLittleEndian() -1;
    IRQ = true;
    NMI.clearInterrupt();
    cycles+=1;
}

void CPU::RTS(){
    spdlog::info("RTS");
    PC = popLittleEndian();
    cycles+=1;
    // PC = PC + 1; //check this
}    

void CPU::PHP(){
    spdlog::info("PHP");
    push(P);
    cycles+=1;
}

void CPU::PLP(){
    spdlog::info("PLP");
    P = pop();
    cycles+=1;
}

void CPU::PHA(){
    spdlog::info("PHA");
    push(A);
    cycles+=1;
}

void CPU::PLA(){
    spdlog::info("PLA");
    A = pop();
    cycles+=1;
}

void CPU::DEY(){
    spdlog::info("DEY");
    Y = Y - 1;
    checkValueFlags(Y);
    cycles+=1;
}

void CPU::TAY(){
    spdlog::info("TAY");
    Y = A;
    checkValueFlags(Y);
    cycles+=1;
}

void CPU::INY(){
    spdlog::info("INY");
    Y = Y + 1;
    checkValueFlags(Y);
    cycles+=1;
}

void CPU::INX(){
    spdlog::info("INX");
    X = X + 1;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::CLC(){
    spdlog::info("CLC");
    setFlag(CARRY, 0);
    cycles+=1;
}
void CPU::SEC(){
    spdlog::info("SEC");
    setFlag(CARRY, 1);
    cycles+=1;
}
void CPU::CLI(){
    spdlog::info("CLI");
    setFlag(INT, 0);
    cycles+=1;
}

void CPU::SEI(){
    spdlog::info("SEI");
    setFlag(INT, 1);
    cycles+=1;
}

void CPU::TYA(){
    spdlog::info("TYA");
    A = Y;
    checkValueFlags(A);
    cycles+=1;
}   

void CPU::CLV(){
    spdlog::info("CLV");
    setFlag(INTEGER_OVERFLOW, 0);
    cycles+=1;
}

void CPU::CLD(){
    spdlog::info("CLD");  
    cycles+=1;
}


void CPU::SED(){
    spdlog::info("SED");
    cycles+=1;
}

void CPU::TXA(){
    spdlog::info("TXA");
    A = X;
    checkValueFlags(A);
    cycles+=1;
}

void CPU::TXS(){
    spdlog::info("TXS");
    SP = X;
    cycles+=1;
}

void CPU::TAX(){
    spdlog::info("TAX");
    X = A;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::TSX(){
    spdlog::info("TSX");
    X = SP;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::DEX(){
    spdlog::info("DEX");
    X = X - 1;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::NOP(){
    spdlog::info("NOP"); 
    cycles+=1;
}
