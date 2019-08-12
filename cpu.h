#include<stdio.h>
#include<iostream>
#include "interrupt.h"
#include "memory.h"

class CPU{
    public:
    char A, X, Y, P, SP; //check check setting of stack pointer
    Memory* memory;
    static enum masks{
        CARRY = 0x01,
        ZERO = 0x02,
        INT = 0x04,
       // DEC = 0x08,
        B = 0x10,
        OVERFLOW = 0x20,
        NEGATIVE = 0x40
    } flagMask;

    unsigned short PC;
    EdgeInterrupt NMI;
    bool IRQ; // refactor
    CPU(Memory* memory);
    void writeAddress(unsigned short address, char value);
    short readLittleEndian(unsigned short address);
    void setFlag(char mask, bool bit);
    bool getFlag(char mask);
    void checkValueFlags(char value);
    void printStatus();
    void readImmediate(unsigned short& PC, char& data, unsigned short address);
    void readZeroPage(unsigned short& PC, char& data, unsigned short address);
    void readAbsolute(unsigned short& PC, char& data, unsigned short &address);
    void readZeroPageX(unsigned short& PC, char& data, unsigned short& address, char X);
    void readAbsoluteX(unsigned short &PC, char& data, unsigned short & address,char X);
    void processInstruction(unsigned char instruction);
    void cycle();
    void OAMDMA(char data);
    void ORA(char data);
    void AND(char data);
    void EOR(char data);
    void ADC(char data);
    void SBC(char data);
    void STA(short int address);
    void LDA(char data);
    void CMP(char data);
    void ASL(char data, unsigned short address, bool accumulator);
    void ROL(char data, unsigned short address, bool accumulator);
    void LSR(char data, unsigned short address, bool accumulator);
    void ROR(char data, unsigned short address, bool accumulator);
    void STX(char data, unsigned short address, bool accumulator);
    void LDX(char data);
    void DEC(char data, unsigned short address, bool accumulator);
    void INC(char data, unsigned short address, bool accumulator);
    void BIT(char data);
    void JMP(unsigned short address);
    void JMP_ABS(unsigned short address);
    void STY(unsigned short address);
    void LDY(char data);
    void CPY(char data);
    void CPX(char data);
    void BRANCH(masks flag, bool bit, char data);
    void push(char data);
    void pushLittleEndian(short data);
    char pop();
    short popLittleEndian();
    void BRK();
    void JSR();
    void RTI();
    void RTS();
    void PHP();
    void PLP();
    void PHA();
    void PLA();
    void DEY();
    void TAY();
    void INY();
    void INX();
    void CLC();
    void SEC();
    void CLI();
    void SEI();
    void TYA();
    void CLV();
    void CLD();
    void SED();
    void TXA();
    void TXS();
    void TAX();
    void TSX();
    void DEX();
    void NOP();
};