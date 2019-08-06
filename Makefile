nesemulator: 
	g++ main.cpp cpu.cpp cartridge.cpp -o cpu -g
	
test:
	g++ tests.cpp cpu.cpp cartridge.cpp -o tests -g