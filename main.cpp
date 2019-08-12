#include "nes.h"
int main(int argc, char* argv[]){
    NES nes(argv[1]);
    nes.start();
}