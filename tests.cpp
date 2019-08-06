#include <bits/stdc++.h>
#include "nes.h"
using namespace std;

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
    cout << nes.A << "\n";
    if (nes.A != 0x8D && nes.getFlag(NES::CARRY) == 0)
        cout << "ROR Error\n";

    // ROR Test- without accumulator
    // nes.ROR('',,false);
}
