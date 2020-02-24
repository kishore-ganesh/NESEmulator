nesemulator: main.cpp cartridge.cpp nes.cpp cpu.cpp memory.cpp interrupt.cpp ppu.cpp sdl_handler.cpp
	g++ main.cpp nes.cpp cpu.cpp cartridge.cpp memory.cpp interrupt.cpp ppu.cpp sdl_handler.cpp -o cpu -g -std=c++17 -pthread -lSDL2
	
test: main.cpp cartridge.cpp nes.cpp cpu.cpp memory.cpp interrupt.cpp ppu.cpp sdl_handler.cpp
	g++ tests.cpp nes.cpp cpu.cpp cartridge.cpp memory.cpp interrupt.cpp ppu.cpp sdl_handler.cpp -o tests -g -std=c++17 -pthread -lSDL2
