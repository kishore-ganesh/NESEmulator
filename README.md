# NESEmulator
An emulator for the Nintendo Entertainment System - A Work in Progress

Over the past few months, I've been working on creating an emulator for the Nintendo Entertainment System, a console from 1984
that singlehandedly brought back the videogame industry from the brinks of extinction.

It has specs that some may consider primitive in our times, with a paltry 2KB of RAM and a 1.79 Mhz 6502 CPU. It was a beast in its own age, however,
with its custom Picture Processing Unit being one of the foremost 2D picture processors of that age.

## Current Status:

Is able to boot up and play Donkey Kong and Super Mario Bros. It is in an early stage, with glitches in rendering. The CPU has been
implemented with all of its instructions, and the baseline model for the PPU is ready. Audio hasn't been implemented yet.

The PPU's scrolling is currently implemented in a crude manner, and needs to be refactored to become more cycle accurate.



## File Structure:


        ├── CMakeLists.txt
        ├── color_structs
        ├── cpu
        ├── include
        │   ├── cartridge.h
        │   ├── controller.h
        │   ├── cpu.h
        │   ├── interrupt.h
        │   ├── memory.h
        │   ├── nes.h
        │   ├── ppu.h
        │   ├── sdl_handler.h
        │   └── util.h
        ├── lib
        ├── Makefile
        ├── rgb_struct_new.py
        ├── rgb_struct.py
        └── src
            ├── cartridge.cpp - Contains code for handling Cartridge Loading
            ├── controller.cpp - Input Handled here
            ├── cpu.cpp - The brains of the 6502 CPU
            ├── interrupt.cpp - Interrupt Functionality
            ├── main.cpp - Driver code
            ├── memory.cpp - Memory Bus (Common between CPU and PPU)
            ├── nes.cpp - Wraps CPU, PPU and Controller
            ├── ppu.cpp - The Picture Processing Unit of the NES
            ├── sdl_handler.cpp - Wrapper for SDL code, the actual rendering happens here
            └── tests.cpp - A few tests (not exhaustive) for testing a few instructions

## Bird's eye view

At every step, the CPU reads the next instruction at the address specified by the Program Counter, executes it, while counting cycles.
The PPU runs 3 cycles for every CPU cycle, and there are a set of common memory locations used for communication between both. The PPU synthesizes an
image from these locations, and outputs an RGB array, which is rendered through SDL.

## PPU Rendering

The PPU fetches a byte from an area known as the nametable, which contains an index into a pattern table. Each entry in the pattern table is of 16 bytes, and specifies an 8x8 pixel pattern.
The NES has a 64 color pallete, out of which 16 can be active at any one time. For indexing 16 palletes, we need four bits, and two come from the pattern table entry. Two more bits come from the screen area's corresponding
attribute table (Each 32x32 part of the screen shares an attribute).

Scrolling is done through xscroll and yscroll registers.


## PPU Registers
PPUCTRL  
PPUPPUMASK   
PPUSTATUS  
OAMADDR  
OAMDATA  
PPUSCROLL  
PPUADDR  
PPUDATA  

