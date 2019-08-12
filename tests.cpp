#include <vector>
#include<iostream>
#include "nes.h"
using namespace std;

void clearFlags(NES& nes){
    vector<short> Flags = {0x01, 0x02, 0x04, 0x10, 0x20, 0x40};
    for (auto flag : Flags)
    {        
            nes.setFlag(flag, 0);
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
            nes.setFlag(flag, boolbit);
            nes.getFlag(flag) == boolbit ? Flagtest = true : Flagtest = false;
            if (!Flagtest)
            {
                cout << "FlagTest Failed:" << flag << "\n";
                break;
            }
        }
    }

    nes.setFlag(NES::CARRY, 1);
    // ROR Test- with accumulator
    nes.A = 0x1A;
    nes.ROR('0', nes.A, true);
    if (!(nes.A == (char)0x8D && nes.getFlag(NES::CARRY) == 0))
        cout << "ROR Error\n";
    clearFlags(nes);    
    nes.writeAddress(0x0712, 0x1A);
    nes.ROR(0x1A, 0x0712, false);

    if (!(nes.readAddress(0x0712) == (char)0x8D && nes.getFlag(NES::CARRY) == 0))
        cout << "ROR Error\n";
    clearFlags(nes);
    nes.checkValueFlags(-1);

    if(!(!nes.getFlag(NES::ZERO)&&nes.getFlag(NES::NEGATIVE))){
        cout << "Check value flags error - Negative";
    }
    clearFlags(nes);
    nes.checkValueFlags(0);
    if(!(nes.getFlag(NES::ZERO)&&!nes.getFlag(NES::NEGATIVE))){
        cout << "Check value flags error - Zero";
    }

    nes.writeAddress(0x0712, 0x04);
    if(nes.readAddress(0x0712)!=0x04){
        cout << "NES write/read error" <<endl;
    }

    clearFlags(nes);
    nes.A = 0x8D;
    nes.BIT(0x8D);
    if(!(nes.getFlag(NES::ZERO)&&nes.getFlag(NES::NEGATIVE) && !nes.getFlag(NES::OVERFLOW))){
        cout << "BIT ERROR" << endl;
    }
    
    /*
    JMP last of page and normal
     */

    nes.writeAddress(0x0712, 0x12);
    nes.writeAddress(0x0713, 0x13);
    if(nes.readLittleEndian(0x0712)!=(unsigned short)(0x1312)){
        cout << "Read little endian error" << endl;
    }
    nes.JMP(0x0712);

    if(nes.PC!=0x1312){
        cout << "JMP normal error" << endl;
    }

    nes.writeAddress(0x0700, 0x01);
    nes.writeAddress(0x07FF, 0x14);
    nes.JMP(0x07FF);
    if(nes.PC!=0x0114){
        cout << "JMP page boundary error" << endl;
    }






    // ROR Test- without accumulator
    // nes.ROR('',,false);
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