
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
/*We should handle addresses automaitcally or not? */
/*Look at best practices and refactor */
//Check write addresses
//Introduce cycle accuracy later
//Implement the B flag
//Delay through Cycles
#include "nes.h"
    
NES::NES(char* path){
    //add CPU code
    memory = new Memory(path);
    cpu = new CPU(memory);
    ppu = new PPU(memory);
}


void NES::start(){
    while(true){
        cpu->cycle();
        cpu->printStatus();
    }
}




//What happens on reset and what happens every cycle
//Check JUMP on edge or what?
//Check reset
//Do CPU and PPU cycle together? Can the CPU and the PPU both do something in the same cycle