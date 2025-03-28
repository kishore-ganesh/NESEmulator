#ifndef NES_INCLUDE
#define NES_INCLUDE
#include "apu.h"
#include "controller.h"
#include "cpu.h"
#include <iostream>
#include <memory>
#include <stdio.h>

class NES {
private:
  coro::event ppuExecutionStopped;
  coro::event ppuCyclesAvailable;

public:
  // check check setting of stack pointer
  std::shared_ptr<Controller> controller;
  std::shared_ptr<APU> apu;
  std::shared_ptr<Memory> memory;
  std::unique_ptr<CPU> cpu;
  std::shared_ptr<PPU> ppu;

  int cpuCycles;
  /*
  A - Accumulator
  X, Y - Index register
  P - Processor Status (flag)
   */
  unsigned short PC;
  bool IRQ, NMI;
  bool previousNMILevel;
  NES(const char *path)
      : controller(std::make_shared<Controller>()),
        apu(std::make_shared<APU>()),
        memory(std::make_shared<Memory>(path, controller, apu)),
        cpu(std::make_unique<CPU>(memory)),
        ppu(std::make_shared<PPU>(memory, cpu->getNMIPointer(),
                                  ppuExecutionStopped, ppuCyclesAvailable)) {
    memory->setPPU(std::weak_ptr(ppu));
  }
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
  void resumePpu() { ppuCyclesAvailable.set(); }
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
};
#endif