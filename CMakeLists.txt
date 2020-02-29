cmake_minimum_required(VERSION 3.15)
include_directories(include)
project("NES Emulator")
find_package(SDL2 REQUIRED)
find_package(spdlog REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
set(SOURCES src/ppu.cpp src/nes.cpp src/cpu.cpp src/main.cpp src/memory.cpp
    src/cartridge.cpp src/sdl_handler.cpp src/interrupt.cpp)
add_executable(nes ${SOURCES})
string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
target_link_libraries(nes ${SDL2_LIBRARIES} spdlog::spdlog)
target_compile_options(nes PRIVATE -Wall)
message(STATUS "CPPFLAGS" ${CMAKE_CPP_FLAGS})

