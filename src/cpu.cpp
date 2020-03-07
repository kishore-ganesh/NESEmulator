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
    shouldCaptureInput = false;
}

EdgeInterrupt* CPU::getNMIPointer(){
    return &NMI;
}

bool CPU::captureInput(){
    return shouldCaptureInput;
}
//Stop input captur
bool CPU::stopCapture(){
    bool shouldStop = stopCaptureInput;
    stopCaptureInput = 0;
    return shouldStop;
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
    SPDLOG_INFO("Program Counter: {0:x}", PC);
    SPDLOG_INFO("Stack pointer: {0:d}", SP);
    SPDLOG_INFO("Registers: A: {0:d}, X: {1:d}, Y: {2:d}", A, X, Y);
    SPDLOG_INFO("Flags: \n N: {0:d} \n Z: {1:d} \n  Carry: {2:d} \n Overflow: {3:d} \n Interrupt Disable: {4:d}\n", \
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
    if(address == 0x4016){
        shouldCaptureInput = value & 0x01;
        stopCaptureInput = !shouldCaptureInput;
        SPDLOG_INFO("SHOULD CAPTURE INPUT: {0:b}",shouldCaptureInput);
    }

    memory->writeAddress(address, value);
}

//Address set to PC for uniform access
void CPU::readImmediate(unsigned short& PC, unsigned short& address){
    SPDLOG_INFO("IMMEDIATE ");
    PC = PC + 1;
    address = PC;
}

void CPU::readZeroPage(unsigned short& PC, unsigned short& address){
    SPDLOG_INFO("ZERO PAGE {0:x}", address);
    PC = PC + 1;
    address = (readAddress(PC))&0x00FF;
    SPDLOG_INFO("ZERO PAGE READ {0:x}", address);
}

void CPU::readAbsolute(unsigned short& PC, unsigned short &address){
    SPDLOG_INFO("ABSOLUTE ");
    PC = PC + 1;
    address = readLittleEndian(PC);
    PC = PC + 1;
}

/* ZERO PAGE, X: X provided since in two instructions the X changes to Y */
void CPU::readZeroPageX(unsigned short& PC, unsigned short& address, unsigned char X){
    SPDLOG_INFO("ZEROPAGE X");
    PC = PC + 1;
    address = (readAddress(PC)+ X)&0x00FF;
    // address = readAddress((0x00FF)&address);
}

void CPU::readAbsoluteX(unsigned short &PC, unsigned short & address,unsigned char X){
    SPDLOG_INFO("ABSOLUTE X ");
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
            case 0x0: BRANCH(NEGATIVE, bit); SPDLOG_INFO("Branch if Negative: {0:b}", bit); break;
            case 0x1: BRANCH(INTEGER_OVERFLOW, bit); SPDLOG_INFO("Branch if Overflow: {0:b}", bit); break;
            case 0x2: BRANCH(CARRY, bit); SPDLOG_INFO("Branch if Carry: {0:b}", bit); break;
            case 0x3: BRANCH(ZERO, bit); SPDLOG_INFO("Branch if Zero: {0:b}", bit); break;
        }
    }
    else if(cc==0x01){
        /* Check addressing modes */
        switch(bbb){
            case 0x0: {
                SPDLOG_INFO("(Indirect, X)") ;
                PC = PC + 1;
                address = readAddress(PC);
                address = (readAddress((address+X+1)&0x00FF)<<8) |readAddress((address+X)&0x00FF);
                // address = readLittleEndian((address+X)&0x00FF);
                // address = addres
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
                SPDLOG_INFO("(Indirect, Y)") ;
                PC = PC + 1;
                address = (readAddress(PC)) & 0x00FF;
                address = ((readAddress((address+1)&0x00FF)<<8) |readAddress(address)) + Y;                
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
        SPDLOG_INFO("NMI Interrupt occured");
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = readLittleEndian(0xFFFA);
        PC = address;
    }
    else if(!IRQ&&!getFlag(INT)){
        SPDLOG_INFO("Interrupt occured");
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
    SPDLOG_INFO("ORA");
    A|=data;
    checkValueFlags(A);
}

void CPU::AND(unsigned short address){
    char data = readAddress(address);
    SPDLOG_INFO("AND");
    A&=data;
    checkValueFlags(A);
}
void CPU::EOR(unsigned short address){
    char data = readAddress(address);
    SPDLOG_INFO("EOR");
    A^=data;
    checkValueFlags(A);
}
void CPU::ADC(unsigned short address){
    unsigned char data = readAddress(address);
    SPDLOG_INFO("ADC");
    SPDLOG_INFO("{0:d}",data);
    short result = A + data+getFlag(CARRY);
    bool carryBit = result > 0xFF ? 1 : 0;
    char charResult = A + (char)data + getFlag(CARRY);
    bool overFlowBit = ((char)A>0&&(char)data>0&&charResult < 0) || ((char)A < 0&& (char)data< 0&&charResult > 0);
    A = A + (char)data + getFlag(CARRY);
    SPDLOG_INFO("A is now: {0:d}",A);
    checkValueFlags(A);
    setFlag(CARRY, carryBit);
    setFlag(INTEGER_OVERFLOW, overFlowBit); //fix this and have carry
}
void CPU::SBC(unsigned short address){
    SPDLOG_INFO("SBC");
    unsigned char data = readAddress(address);
    short result = A - data - !getFlag(CARRY);
    char charResult = A - (char)data - !getFlag(CARRY);
    bool overFlowBit = ((char)A > 0 && (char)data < 0 && charResult < 0) || ((char)A < 0 && (char)data > 0 && charResult > 0);
    SPDLOG_INFO("{0:d}",data);
    // bool borrowBit = A > data ? 0;
    //Short won't show overflow
    unsigned char cmpData = data + !getFlag(CARRY);
    if(A < cmpData){
        setFlag(CARRY, 0);
    }
    else{
        setFlag(CARRY, 1);
    }
    A = A - cmpData;
    SPDLOG_INFO("A is now: {0:d}",A);
    checkValueFlags(A);
    setFlag(INTEGER_OVERFLOW, overFlowBit);
    
    
}
void CPU::STA(unsigned short address){
    SPDLOG_INFO("STA");
    writeAddress(address, A);  // check this
}
void CPU::LDA(unsigned short address){
    unsigned char data = readAddress(address);
    SPDLOG_INFO("LDA {0:x}", address);
    A = data; //check if flag is to be set here
    checkValueFlags(A);
}
void CPU::CMP(unsigned short address){
    unsigned char data = readAddress(address);
    SPDLOG_INFO("CMP A: {0:d} & Data: {1:d}", A, data);
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
    SPDLOG_INFO("ASL");
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
    SPDLOG_INFO("ROL");
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
    SPDLOG_INFO("LSR");
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
    SPDLOG_INFO("ROR");
}
void CPU::STX(unsigned short address, bool accumulator){
    writeAddress(address, X);
    SPDLOG_INFO("STX"); //check if flag is set here
}

void CPU::LDX(unsigned short address){
    unsigned char data = readAddress(address);
    SPDLOG_INFO("LDX");
    X = data; //check if flag to be set here
    checkValueFlags(X);
}
void CPU::DEC(unsigned short address, bool accumulator){
    unsigned char data = readAddress(address);
    writeAddress(address, data - 1);
    checkValueFlags(data -1);
    SPDLOG_INFO("DEC");
}
void CPU::INC(unsigned short address, bool accumulator){
    unsigned char data = readAddress(address);
    writeAddress(address, data + 1);
    checkValueFlags(data + 1);
    SPDLOG_INFO("INC");
}

void CPU::BIT(unsigned short address){
    // Check for immediate instruction
    SPDLOG_INFO("BIT");
    if(address == PC){
        SPDLOG_INFO("BIT IMMEDIATE");
    }
    unsigned char data = readAddress(address);
    bool zeroBit, overflowBit, negativeBit;
    zeroBit = data & A == 0 ? 1 : 0;
    negativeBit = data & 0x80;
    overflowBit = (data & 0x40);
    SPDLOG_INFO("BIT Data is {}", data);
    SPDLOG_INFO("BIT negativeBit: {0:b}, overflowBit: {0:d}", negativeBit, 0x8D & 0x40);
    setFlag(ZERO, (data & A)==0);
    setFlag(INTEGER_OVERFLOW, overflowBit);
    setFlag(NEGATIVE, negativeBit);
}

void CPU::JMP(unsigned short address){
    SPDLOG_INFO("JMP");
    if(!((address&0x00FF)==0x00FF)){
        PC = readLittleEndian(address) - 1;
    }
    else{
        short jumpTo = (readAddress(address&0xFF00) << 8) | (readAddress(address));
        PC = jumpTo - 1 ;
    }
    
}

void CPU::JMP_ABS(unsigned short address){
    SPDLOG_INFO("JMP_ABS");
    PC = address - 1;

    //check for page boundaries
}

void CPU::STY(unsigned short address){
    SPDLOG_INFO("STY");
    writeAddress(address, Y);
}

void CPU::LDY(unsigned short address){
    unsigned char data = readAddress(address);
    SPDLOG_INFO("LDY");
    Y = data;
    checkValueFlags(data);
}

void CPU::CPY(unsigned short address){
    SPDLOG_INFO("CPY");
    unsigned char data = readAddress(address);
    bool borrowBit = Y >= data ? 1: 0;
    bool zeroBit = false;
    if(Y==data){
        zeroBit = true;
    }
    setFlag(CARRY, borrowBit);
    checkValueFlags(Y - data);

}
void CPU::CPX(unsigned short address){
    unsigned char data = readAddress(address);
    SPDLOG_INFO("CPX");
    bool borrowBit = X >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(X - data);
}

void CPU::BRANCH(masks flag, bool bit){
    PC = PC + 1;
    char data = readAddress(PC);
    // SPDLOG_INFO("BRANCH");
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
    SPDLOG_INFO("Popping: {0:x}", data);
    return data;
}

short CPU::popLittleEndian(){
    short result = (unsigned char) pop();
    result = (((unsigned char)pop() )<< 8) | result;
    return result;
}

void CPU::BRK(){
    SPDLOG_INFO("BRK");
    setFlag(INT, 1);
    IRQ = false;
    pushLittleEndian(PC+2);
    push(P);
    cycles+=1;
}

void CPU::JSR(){
    SPDLOG_INFO("JSR");
    pushLittleEndian(PC+2);
    PC = PC + 1;
    short address = readLittleEndian(PC);
    address = address - 1;
    PC = address;
    //check correct PC behavior
}

void CPU::RTI(){
    SPDLOG_INFO("RTI");
    P = pop();
    PC = popLittleEndian() -1;
    IRQ = true;
    NMI.clearInterrupt();
    cycles+=1;
}

void CPU::RTS(){
    SPDLOG_INFO("RTS");
    PC = popLittleEndian();
    cycles+=1;
    // PC = PC + 1; //check this
}    

void CPU::PHP(){
    SPDLOG_INFO("PHP");
    push(P);
    cycles+=1;
}

void CPU::PLP(){
    SPDLOG_INFO("PLP");
    P = pop();
    cycles+=1;
}

void CPU::PHA(){
    SPDLOG_INFO("PHA");
    push(A);
    cycles+=1;
}

void CPU::PLA(){
    SPDLOG_INFO("PLA");
    A = pop();
    checkValueFlags(A);
    cycles+=1;
}

void CPU::DEY(){
    SPDLOG_INFO("DEY");
    Y = Y - 1;
    checkValueFlags(Y);
    cycles+=1;
}

void CPU::TAY(){
    SPDLOG_INFO("TAY");
    Y = A;
    checkValueFlags(Y);
    cycles+=1;
}

void CPU::INY(){
    SPDLOG_INFO("INY");
    Y = Y + 1;
    checkValueFlags(Y);
    cycles+=1;
}

void CPU::INX(){
    SPDLOG_INFO("INX");
    X = X + 1;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::CLC(){
    SPDLOG_INFO("CLC");
    setFlag(CARRY, 0);
    cycles+=1;
}
void CPU::SEC(){
    SPDLOG_INFO("SEC");
    setFlag(CARRY, 1);
    cycles+=1;
}
void CPU::CLI(){
    SPDLOG_INFO("CLI");
    setFlag(INT, 0);
    cycles+=1;
}

void CPU::SEI(){
    SPDLOG_INFO("SEI");
    setFlag(INT, 1);
    cycles+=1;
}

void CPU::TYA(){
    SPDLOG_INFO("TYA");
    A = Y;
    checkValueFlags(A);
    cycles+=1;
}   

void CPU::CLV(){
    SPDLOG_INFO("CLV");
    setFlag(INTEGER_OVERFLOW, 0);
    cycles+=1;
}

void CPU::CLD(){
    SPDLOG_INFO("CLD");  
    cycles+=1;
}


void CPU::SED(){
    SPDLOG_INFO("SED");
    cycles+=1;
}

void CPU::TXA(){
    SPDLOG_INFO("TXA");
    A = X;
    checkValueFlags(A);
    cycles+=1;
}

void CPU::TXS(){
    SPDLOG_INFO("TXS");
    SP = X;
    cycles+=1;
}

void CPU::TAX(){
    SPDLOG_INFO("TAX");
    X = A;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::TSX(){
    SPDLOG_INFO("TSX");
    X = SP;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::DEX(){
    SPDLOG_INFO("DEX");
    X = X - 1;
    checkValueFlags(X);
    cycles+=1;
}

void CPU::NOP(){
    SPDLOG_INFO("NOP"); 
    cycles+=1;
}
