nesemulator: main.cpp cartridge.cpp cpu.cpp
	g++ main.cpp cpu.cpp cartridge.cpp -o cpu -g -std=c++11
	
test: main.cpp cartridge.cpp cpu.cpp
	g++ tests.cpp cpu.cpp cartridge.cpp -o tests -g -std=c++11
