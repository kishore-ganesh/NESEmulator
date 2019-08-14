
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
/*We should handle addresses automaitcally or not? */
/*Look at best practices and refactor */
//Check write addresses
//Introduce cycle accuracy later
//Implement the B flag
//Delay through Cycles
#include "nes.h"
#include <thread>
    
NES::NES(char* path){
    //add CPU code
    memory = new Memory(path);
    cpu = new CPU(memory);
    ppu = new PPU(memory);
}


void NES::start(){
    auto cpuLoop = [](CPU* cpu){
        while(true){
            cpu->cycle();
            cpu->printStatus();
        }
    };

    auto ppuLoop = [](PPU* ppu){
        while(true){
            ppu->generateFrame();
            ppu->displayFrame();
        }
    };
    std::thread cpuThread(cpuLoop, cpu);
    std::thread ppuThread(ppuLoop, ppu);
    cpuThread.join();
    ppuThread.join();
}




//What happens on reset and what happens every cycle
//Check JUMP on edge or what?
//Check reset
//Do CPU and PPU cycle together? Can the CPU and the PPU both do something in the same cycle