#ifndef NES_INCLUDE
#define NES_INCLUDE
#include "apu.h"
#include "controller.h"
#include "cpu.h"
#include <iostream>
#include <stdio.h>

class NES {
public:
  // check check setting of stack pointer
  CPU *cpu;
  Memory *memory;
  PPU *ppu;
  APU *apu;
  Controller *controller;
  int cpuCycles;
  /*
  A - Accumulator
  X, Y - Index register
  P - Processor Status (flag)
   */
  unsigned short PC;
  bool IRQ, NMI;
  bool previousNMILevel;
  NES(char *path);
  void cpuCycle();
  coro::task<void> ppuCycle();
  bool ppuCyclesLeft();
  bool apuCyclesLeft();
  void apuCycle();
  bool shouldRender();
  void setTime(unsigned int delta);
  bool hasCPUCycles();
  std::vector<std::vector<RGB>> getFrame();

  bool ppuCanExecute() { return ppu->canExecute(); }
  void raisePpuShouldQuit() { ppu->raiseShouldQuit(); };
  void resumePpu() {
    ppuCyclesAvailable.set();
  }
  void resumePpuIfPossible() {
    if (ppu->canExecute()) {
      ppuCyclesAvailable.set();
    }
  }

  coro::task<void> waitForPpuExecution() {
    co_await ppuExecutionStopped;
    // TODO: is this necessary
    ppuExecutionStopped.reset();
  }

private:
  coro::event ppuExecutionStopped;
  coro::event ppuCyclesAvailable;
};
#endif