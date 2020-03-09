#include<stdio.h>
#include<iostream>
#include "interrupt.h"
#include "memory.h"


class CPU{
    public:
    unsigned char A, X, Y, P; //check check setting of stack pointer
    unsigned char SP;
    int cycles;
    Memory* memory;
    bool shouldCaptureInput;
    bool stopCaptureInput;
    enum masks{
        CARRY = 0x01,
        ZERO = 0x02,
        INT = 0x04,
        DECIMAL = 0x08,
        B = 0x10,
        INTEGER_OVERFLOW = 0x40,
        NEGATIVE = 0x80
    } flagMask;

    unsigned short PC;
    EdgeInterrupt NMI;
    bool IRQ; // refactor
    CPU(Memory* memory);
    EdgeInterrupt* getNMIPointer();
    unsigned char readAddress(unsigned short address);
    void writeAddress(unsigned short address, char value);
    short readLittleEndian(unsigned short address);
    bool captureInput();
    bool stopCapture();
    void setFlag(char mask, bool bit);
    bool getFlag(char mask);
    void checkValueFlags(char value);
    void printStatus();
    void readImmediate(unsigned short& PC, unsigned short &address);
    void readZeroPage(unsigned short& PC, unsigned short &address);
    void readAbsolute(unsigned short& PC, unsigned short &address);
    void readZeroPageX(unsigned short& PC, unsigned short& address, unsigned char X);
    void readAbsoluteX(unsigned short &PC, unsigned short & address,unsigned char X);
    void processInstruction(unsigned char instruction);
    int cycle();
    void OAMDMA(char data);
    void ORA(unsigned short address);
    void AND(unsigned short address);
    void EOR(unsigned short address);
    void ADC(unsigned short address);
    void SBC(unsigned short address);
    void STA(unsigned short address);
    void LDA(unsigned short address);
    void CMP(unsigned short address);
    void ASL(unsigned short address, bool accumulator);
    void ROL(unsigned short address, bool accumulator);
    void LSR(unsigned short address, bool accumulator);
    void ROR(unsigned short address, bool accumulator);
    void STX(unsigned short address, bool accumulator);
    void LDX(unsigned short address);
    void DEC(unsigned short address, bool accumulator);
    void INC(unsigned short address, bool accumulator);
    void BIT(unsigned short address);
    void JMP(unsigned short address);
    void JMP_ABS(unsigned short address);
    void STY(unsigned short address);
    void LDY(unsigned short address);
    void CPY(unsigned short address);
    void CPX(unsigned short address);
    void BRANCH(masks flag, bool bit);
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