nesemulator: main.cpp cartridge.cpp nes.cpp cpu.cpp memory.cpp interrupt.cpp ppu.cpp
	g++ main.cpp nes.cpp cpu.cpp cartridge.cpp memory.cpp interrupt.cpp ppu.cpp -o cpu -g -std=c++11 -pthread -lSDL2
	
test: main.cpp cartridge.cpp nes.cpp cpu.cpp memory.cpp interrupt.cpp ppu.cpp
	g++ tests.cpp nes.cpp cpu.cpp cartridge.cpp memory.cpp interrupt.cpp ppu.cpp -o tests -g -std=c++11 -pthread -lSDL2
