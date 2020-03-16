
/* RUN SET FLAGS AFTER RUNNING INSTRUCTION */
/*We should handle addresses automaitcally or not? */
/*Look at best practices and refactor */
//Check write addresses
//Introduce cycle accuracy later
//Implement the B flag
//Delay through Cycles
#include "nes.h"
#include <thread>

NES::NES(char *path)
{
    //add CPU code
    controller = new Controller();
    apu = new APU();
    memory = new Memory(path, controller, apu);
    cpu = new CPU(memory);
    ppu = new PPU(memory, cpu->getNMIPointer());
    memory->setPPU(ppu);
}

void NES::cycle()
{

    int cpuCycles = cpu->cycle();
    cpu->printStatus();
    ppu->generateFrame(cpuCycles * 3);
    // if (ppu->shouldRender())
    // {
    //     ppu->displayFrame();
    // }
}

bool NES::shouldRender(){
    return ppu->shouldRender();
}

void NES::ppuCycle(){
    ppu->generateFrame(0);
}

void NES::cpuCycle(){
    cpuCycles = cpu->cycle();
    cpu->printStatus();
    if(cpu->captureInput()){
        controller->capture();
    }
    else if(cpu->stopCapture()){
        controller->stopCapture();
    }
    ppu->addCPUCycles(cpuCycles);
    apu->addCPUCycles(cpuCycles);
}

bool NES::apuCyclesLeft(){
    return apu->getCyclesLeft();
}

void NES::apuCycle(){
    apu->cycle();
}
bool NES::ppuCyclesLeft(){
    return ppu->getCyclesLeft();
}

std::vector<std::vector<RGB>> NES::getFrame(){
    return ppu->getFrame();
}
// auto cpuLoop = [&](CPU* cpu){
//     while(true){
//         cpu->cycle();
//         cpu->printStatus();
//     }
// };

// auto ppuLoop = [&](PPU* ppu){
//     while(true){
//         ppu->generateFrame();
//         ppu->displayFrame();
//     }
// };
// std::thread cpuThread(cpuLoop, cpu);
// std::thread ppuThread(ppuLoop, ppu);
// cpuThread.join();
// ppuThread.join();


//What happens on reset and what happens every cycle
//Check JUMP on edge or what?
//Check reset
//Do CPU and PPU cycle together? Can the CPU and the PPU both do something in the same cycle