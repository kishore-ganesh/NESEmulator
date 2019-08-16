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
    printf("Program Counter: %x\n", PC);
    printf("Stack pointer: %d", SP);
    printf("Registers: A: %d, X: %d, Y: %d\n", A, X, Y);
    printf("Flags: \n N: %d \n Z: %d \n  Carry: %d \n Overflow: %d \n Interrupt Disable: %d\n", \
    getFlag(NEGATIVE),\
    getFlag(ZERO), \
    getFlag(CARRY), \
    getFlag(INTEGER_OVERFLOW), \
    getFlag(INT) \
    );
    printf("IRQ: %d, NMI: %d\n\n", IRQ, NMI.checkInterrupt());
}


void CPU::readImmediate(unsigned short& PC, char& data, unsigned short address){
    cout << "IMMEDIATE ";
    PC = PC + 1;
    address = PC;
    data = memory->readAddress(PC);
}

void CPU::readZeroPage(unsigned short& PC, char& data, unsigned short address){
    cout << "ZERO PAGE ";
    PC = PC + 1;
    address = ((0x00)<<8)|memory->readAddress(PC);
    data = memory->readAddress(address);
}

void CPU::readAbsolute(unsigned short& PC, char& data, unsigned short &address){
    cout << "ABSOLUTE ";
    PC = PC + 1;
    address = memory->readLittleEndian(PC);
    PC = PC + 1;
    data = memory->readAddress(address);
}

/* ZERO PAGE, X: X provided since in two instructions the X changes to Y */
void CPU::readZeroPageX(unsigned short& PC, char& data, unsigned short& address, char X){
    cout << "ZEROPAGE X" ;
    PC = PC + 1;
    address = PC + X;
    data = memory->readAddress(PC+X);
}

void CPU::readAbsoluteX(unsigned short &PC, char& data, unsigned short & address,char X){
    cout <<"ABSOLUTE X ";
    PC = PC + 1;
    address = memory->readLittleEndian(PC) + X;
    PC = PC + 1;
    data = memory->readAddress(address);
}
void CPU::processInstruction(unsigned char instruction){
    
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
            case 0x0: BRANCH(NEGATIVE, bit, data); break;
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
                short address = memory->readAddress(PC);
                address = memory->readLittleEndian(address) + Y;
                data = memory->readAddress(address);                
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
void CPU::cycle(){
    short address;
    if(NMI.checkInterrupt()){
        cout << "NMI Interrupt occured" << endl;
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = memory->readLittleEndian(0xFFFA);
        PC = address;
    }
    else if(!IRQ&&!getFlag(INT)){
        pushLittleEndian(PC);
        push(P);
        setFlag(INT, 1);
        address = memory->readLittleEndian(0xFFFE);
        PC = address; 
    }
    char instruction = memory->readAddress(PC);
    processInstruction(instruction);
    PC = PC + 1; //check for jump
}
void CPU::ORA(char data){
    cout << "ORA" << endl;
    A|=data;
    checkValueFlags(A);
}

void CPU::AND(char data){
    cout << "AND" << endl;
    A&=data;
    checkValueFlags(A);
}
void CPU::EOR(char data){
    cout << "EOR" << endl;
    A^=data;
    checkValueFlags(A);
}
void CPU::ADC(char data){
    cout<< "ADC" << endl;
    bool carryBit = (A + data) > 0xFF ? 1 : 0;
    bool overFlowBit = (A + data > 0x7F || A + data < 0x80);
    setFlag(CARRY, carryBit);
    A+=data;
    checkValueFlags(A);
    setFlag(INTEGER_OVERFLOW, overFlowBit); //fix this and have carry
}
void CPU::SBC(char data){
    cout << "SBC" <<endl;
    bool overFlowBit = (A - data > 0x7F || A - data < 0x80);
    bool borrowBit = A >= data ? 1: 0;
    A-=data;
    checkValueFlags(A);
    setFlag(INTEGER_OVERFLOW, overFlowBit);
    setFlag(CARRY, borrowBit);
}
void CPU::STA(short int address){
    cout << "STA" <<endl;
    memory->writeAddress(address, A);  // check this
}
void CPU::LDA(char data){
    cout << "LDA" <<endl;
    A = data; //check if flag is to be set here
    checkValueFlags(A);
}
void CPU::CMP(char data){
    cout << "CMP" << endl;
    bool borrowBit = A >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(A - data);
    // Update flags based on CMP
}

void CPU::ASL(char data, unsigned short address, bool accumulator){
    bool carryBit = 0;
    if(accumulator){
        carryBit = (A & 0x80) >> 7;
        A = A << 1;
        checkValueFlags(A);
        
    }
    else{
        carryBit = (data & 0x80) >> 7;
        memory->writeAddress(address, data << 1);
        checkValueFlags(data);
    }
    setFlag(CARRY, carryBit);
    cout << "ASL" <<endl;
}
void CPU::ROL(char data, unsigned short address, bool accumulator){
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
        memory->writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, carryBit);
    cout << "ROL" << endl;
}
void CPU::LSR(char data, unsigned short address, bool accumulator){
    bool nextCarryBit = 0;
    if(accumulator){
        nextCarryBit = A & 0x01;
        A = A >> 1;
        checkValueFlags(A);
    }
    else{
        nextCarryBit = data & 0x01;
        data = data >> 1;
        memory->writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, nextCarryBit);
    cout << "LSR" << endl;
}
void CPU::ROR(char data, unsigned short address, bool accumulator){
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
        memory->writeAddress(address, data);
        checkValueFlags(data);
    }
    setFlag(CARRY, nextCarryBit);
    cout << "ROR" << endl;
}
void CPU::STX(char data, unsigned short address, bool accumulator){
    memory->writeAddress(address, X);
    cout << "STX" << endl; //check if flag is set here
}

void CPU::LDX(char data){
    cout<< "LDX" <<endl;
    X = data; //check if flag to be set here
    checkValueFlags(X);
}
void CPU::DEC(char data, unsigned short address, bool accumulator){
    memory->writeAddress(address, data - 1);
    checkValueFlags(data -1);
    cout << "DEC" <<endl;
}
void CPU::INC(char data, unsigned short address, bool accumulator){
    memory->writeAddress(address, data + 1);
    checkValueFlags(data + 1);
    cout << "INC" << endl;
}

void CPU::BIT(char data){
    // Check for immediate instruction
    cout << "BIT" << endl;
    bool zeroBit, overflowBit, negativeBit;
    zeroBit = data & A > 0 ? 1 : 0;
    negativeBit = data & 0x80;
    overflowBit = data & 0x70;
    setFlag(ZERO, data & A);
    setFlag(INTEGER_OVERFLOW, overflowBit);
    setFlag(NEGATIVE, negativeBit);
}

void CPU::JMP(unsigned short address){
    cout << "JMP" << endl;
    if(!((address&0x00FF)==0x00FF)){
        PC = memory->readLittleEndian(address);
    }
    else{
        short jumpTo = (memory->readAddress(address&0xFF00) << 8) | (memory->readAddress(address));
        PC = jumpTo;
    }
    
}

void CPU::JMP_ABS(unsigned short address){
    cout << "JMP_ABS" << endl;
    PC = address - 1;

    //check for page boundaries
}

void CPU::STY(unsigned short address){
    cout << "STY" << endl;
    memory->writeAddress(address, Y);
}

void CPU::LDY(char data){
    cout << "LDY" << endl;
    Y = data;
    checkValueFlags(data);
}

void CPU::CPY(char data){
    cout << "CPY" << endl;
    bool borrowBit = Y >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(Y - data);

}
void CPU::CPX(char data){
    cout << "CPX" << endl;
    bool borrowBit = X >= data ? 1: 0;
    setFlag(CARRY, borrowBit);
    checkValueFlags(X - data);
}

void CPU::BRANCH(masks flag, bool bit, char data){
    cout << "BRANCH" << endl;
    if(getFlag(flag)==bit){
        PC = PC + data - 2; //chedck this
    }
}
void CPU::push(char data){
    unsigned char highByte = 0x01;
    short stackAddress = (highByte << 8) | (unsigned char)SP;
    memory->writeAddress(stackAddress, data);
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
    char data = memory->readAddress(stackAddress);
    return data;
}

short CPU::popLittleEndian(){
    short result = pop();
    result = (pop() << 8) | result;
    return result;
}

void CPU::BRK(){
    cout << "BRK" << endl;
    setFlag(INT, 1);
    IRQ = false;
    pushLittleEndian(PC+2);
    push(P);
}

void CPU::JSR(){
    cout << "JSR" << endl;
    pushLittleEndian(PC+2);
    PC = PC + 1;
    short address = memory->readLittleEndian(PC);
    address = address - 1;
    PC = address;
    //check correct PC behavior
}

void CPU::RTI(){
    cout << "RTI" << endl;
    P = pop();
    PC = popLittleEndian();
    IRQ = true;
    NMI.clearInterrupt();
}

void CPU::RTS(){
    cout << "RTS" << endl;
    PC = popLittleEndian();
    // PC = PC + 1; //check this
}    

void CPU::PHP(){
    cout << "PHP" << endl;
    push(P);
}

void CPU::PLP(){
    cout << "PLP" << endl;
    P = pop();
}

void CPU::PHA(){
    cout << "PHA" << endl;
    push(A);
}

void CPU::PLA(){
    cout << "PLA" << endl;
    A = pop();
}

void CPU::DEY(){
    cout << "DEY" << endl;
    Y = Y - 1;
    checkValueFlags(Y);
}

void CPU::TAY(){
    cout << "TAY" << endl;
    Y = A;
    checkValueFlags(Y);
}

void CPU::INY(){
    cout << "INY" << endl;
    Y = Y + 1;
    checkValueFlags(Y);
}

void CPU::INX(){
    cout << "INX" << endl;
    X = X + 1;
    checkValueFlags(X);
}

void CPU::CLC(){
    cout << "CLC" << endl;
    setFlag(CARRY, 0);
}
void CPU::SEC(){
    cout << "SEC" << endl;
    setFlag(CARRY, 1);
}
void CPU::CLI(){
    cout << "CLI" << endl;
    setFlag(INT, 0);
}

void CPU::SEI(){
    cout << "SEI" << endl;
    setFlag(INT, 1);
}

void CPU::TYA(){
    cout << "TYA" << endl;
    A = Y;
    checkValueFlags(A);
}   

void CPU::CLV(){
    cout << "CLV" << endl;
    setFlag(INTEGER_OVERFLOW, 0);
}

void CPU::CLD(){
    cout << "CLD" << endl;
}


void CPU::SED(){
    cout << "SED" << endl;
}

void CPU::TXA(){
    cout << "TXA" << endl;
    A = X;
    checkValueFlags(A);
}

void CPU::TXS(){
    cout << "TXS" << endl;
    SP = X;
}

void CPU::TAX(){
    cout << "TAX" << endl;
    X = A;
}

void CPU::TSX(){
    cout << "TSX" << endl;
    X = SP;
}

void CPU::DEX(){
    cout << "DEX" << endl;
    X = X - 1;
}

void CPU::NOP(){
    cout << "NOP" << endl; 
}
