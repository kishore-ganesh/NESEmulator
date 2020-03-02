#include <vector>
#include<iostream>
#include "nes.h"
using namespace std;

void clearFlags(NES& nes){
    vector<short> Flags = {0x01, 0x02, 0x04, 0x10, 0x20, 0x40};
    for (auto flag : Flags)
    {        
            nes.cpu->setFlag(flag, 0);
    }
}

void ADC(NES& nes, int a, int b){
    nes.cpu->A = a;
    nes.cpu->writeAddress(0x0700, b);
    nes.cpu->ADC(0x0700);
}

void SBC(NES& nes, int a, int b){
    clearFlags(nes);
    nes.cpu->A = a;
    nes.cpu->writeAddress(0x0700, b);
    nes.cpu->SBC(0x0700);
}

void ADCTest(NES& nes){
    unsigned char firstOps[] = {80, 80, 80,80, 208, 208, 208, 208};
    unsigned char secondOps[] = {16, 80, 144, 208, 16, 80, 144, 208};
    bool carryRes[] = {false, false, false, true, false, true, true, true};
    bool overFlowRes[] = {false, true, false, false, false, false, true, false};
    clearFlags(nes);
    for(int i = 0; i < 8; i++){
        ADC(nes, firstOps[i], secondOps[i]);
        if(nes.cpu->getFlag(CPU::CARRY)!=carryRes[i]){
            printf("INCORRECT CARRY FLAG ADC\n");
            break;
        }
        if(nes.cpu->getFlag(CPU::INTEGER_OVERFLOW)!=overFlowRes[i]){
            printf("INCORRECT OVERFLOW FLAG ADC\n");
            break;
        }
        clearFlags(nes);
    }
}

void SBCTest(NES& nes){
    unsigned char firstOps[] = {80, 80, 80,80, 208, 208, 208, 208};
    unsigned char secondOps[] = {240, 176, 112, 48, 240, 176, 112, 48};
    bool carryRes[] = {false, false, false, true, false, true, true, true};
    bool overFlowRes[] = {false, true, false, false, false, false, true, false};
    clearFlags(nes);
    for(int i = 0; i < 8; i++){
        SBC(nes, firstOps[i], secondOps[i]);
        if(nes.cpu->getFlag(CPU::CARRY)!=carryRes[i]){
            printf("INCORRECT CARRY FLAG SBC\n");
            break;
        }
        if(nes.cpu->getFlag(CPU::INTEGER_OVERFLOW)!=overFlowRes[i]){
            printf("INCORRECT OVERFLOW FLAG SBC\n");
            break;
        }
        clearFlags(nes);
    }
}

void CMPTest(NES& nes){
    nes.cpu->A = 1;
    nes.cpu->writeAddress(0x0700, 0xFF);
    nes.cpu->CMP(0x0700);
    if(nes.cpu->getFlag(CPU::CARRY)){
        printf("INCORRECT CMP CARRY\n");
        return;
    }
    if(nes.cpu->getFlag(CPU::NEGATIVE)){
        printf("INCORRECT CMP NEGATIVE\n");
        return;
    }

    nes.cpu->A = 0x7F;
    nes.cpu->writeAddress(0x0700, 0x80);
    nes.cpu->CMP(0x0700);
    if(nes.cpu->getFlag(CPU::CARRY)){
        printf("INCORRECT CMP CARRY\n");
        return;
    }
    if(!nes.cpu->getFlag(CPU::NEGATIVE)){
        printf("INCORRECT CMP NEGATIVE\n");
        return;
    }

}

int main(int argc, char *argv[])
{
    NES nes(argv[1]);

    // All Flags Tests
    vector<short> Flags = {0x01, 0x02, 0x04, 0x10, 0x20, 0x40};
    for (auto flag : Flags)
    {
        bool Flagtest = false;
        for (int boolbit = 0; boolbit <= 1; boolbit++)
        {
            nes.cpu->setFlag(flag, boolbit);
            nes.cpu->getFlag(flag) == boolbit ? Flagtest = true : Flagtest = false;
            if (!Flagtest)
            {
                cout << "FlagTest Failed:" << flag << "\n";
                break;
            }
        }
    }

    nes.cpu->setFlag(CPU::CARRY, 1);
    // ROR Test- with accumulator
    nes.cpu->A = 0x1A;
    nes.cpu->ROR(nes.cpu->A, true);
    if (!(nes.cpu->A == (char)0x8D && nes.cpu->getFlag(CPU::CARRY) == 0))
        cout << "ROR Error\n"; 
    clearFlags(nes);    
    nes.cpu->memory->writeAddress(0x0712, 0x1A);
    nes.cpu->setFlag(CPU::CARRY, 1);
    nes.cpu->ROR(0x0712, false);
    if (!(nes.cpu->memory->readAddress(0x0712) == 0x8D && nes.cpu->getFlag(CPU::CARRY) == 0))
        cout << "ROR Error\n";
    clearFlags(nes);
    nes.cpu->checkValueFlags(-1);

    if(!(!nes.cpu->getFlag(CPU::ZERO)&&nes.cpu->getFlag(CPU::NEGATIVE))){
        cout << "Check value flags error - Negative";
    }
    clearFlags(nes);
    nes.cpu->checkValueFlags(0);
    if(!(nes.cpu->getFlag(CPU::ZERO)&&!nes.cpu->getFlag(CPU::NEGATIVE))){
        cout << "Check value flags error - Zero";
    }

    nes.cpu->memory->writeAddress(0x0712, 0x04);
    if(nes.cpu->memory->readAddress(0x0712)!=0x04){
        cout << "NES write/read error" <<endl;
    }

    clearFlags(nes);
    nes.cpu->A = 0x8D;
    nes.cpu->writeAddress(0x0700,0x8D);
    nes.cpu->BIT(0x0700);
    if(!(!nes.cpu->getFlag(CPU::ZERO)&&nes.cpu->getFlag(CPU::NEGATIVE) && !nes.cpu->getFlag(CPU::INTEGER_OVERFLOW))){
        cout << "BIT ERROR" << endl;
    }
    
    /*
    JMP last of page and normal
     */

    nes.cpu->memory->writeAddress(0x0712, 0x12);
    nes.cpu->memory->writeAddress(0x0713, 0x13);
    if(nes.cpu->memory->readLittleEndian(0x0712)!=(unsigned short)(0x1312)){
        cout << "Read little endian error" << endl;
    }
    nes.cpu->JMP(0x0712);

    if(nes.cpu->PC!=0x1311){
        cout << "JMP normal error" << endl;
    }

    nes.cpu->memory->writeAddress(0x0700, 0x01);
    nes.cpu->memory->writeAddress(0x07FF, 0x14);
    nes.cpu->JMP(0x07FF);
    if(nes.cpu->PC!=0x0113){
        cout << "JMP page boundary error" << endl;
    }

    char push_test[] = {0, 1, 2, 3, 4};
    for(int i=0; i<5; i++){
        nes.cpu->push(push_test[i]);
    }
    for(int i=4; i>=0; i--){
        if(nes.cpu->pop()!=push_test[i]){
            cout << "STACK TEST FAILED" << endl;
            break;
        }
    }

    nes.cpu->SP = 0x1F;
    short push_test_short[] = {0, 1, 2,3, 4};
    for(int i= 0; i<5; i++){
        nes.cpu->pushLittleEndian(push_test_short[i]);
    }
    for(int i=4; i>=0; i--){
        if(nes.cpu->popLittleEndian()!=push_test_short[i]){
            cout << "STACK TEST FAILED" << endl;
            break;
        }
    }

    // Render tests
    for(int i=0; i<256; i++){
        for(int j=0; j<240; j++){
            RGB red{255, 0, 0};
            nes.ppu->setPixel(i, j, red);
        }
        
    }
    // while(true){
    //     nes.ppu->displayFrame();
    // }

    nes.cpu->A = -1;
    nes.cpu->LSR(0, true);
    if(nes.cpu->A!=0x7F){
        cout << "LSR sign error" << endl;
    }

    ADCTest(nes);

    SBCTest(nes);
    CMPTest(nes);
    




    // ROR Test- without accumulator
    // nes.cpu->ROR('',,false);
}



/*
Instruction testing

 */

 /*
 Addressing mode testing
  */

  /*
  Calling instructions tesitng
   */

  /*Whether we check unsigned */