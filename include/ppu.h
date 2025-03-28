#ifndef PPU_INCLUDE
#define PPU_INCLUDE
#include "interrupt.h"
#include "memory.h"
#include "util.h"

#include <coro/coro.hpp>
#include <vector>

// Nameppaces

class Memory; // why did forward declaration work
enum class Registers {
  PPUCTRL,
  PPUMASK,
  PPUSTATUS,
  OAMADDR,
  OAMDATA,
  PPUSCROLL,
  PPUADDR,
  PPUDATA,
};

struct Sprite {
  uint8_t y;
  uint8_t tileIndex;
  uint8_t attributes;
  uint8_t x;
  uint8_t index;
  void print() {
    SPDLOG_INFO("SPRITE at {0:d}, {1:d} with tileIndex {2:d}", x, y, tileIndex);
  }
};

struct SpritePPUInfo {
  uint8_t upperPattern;
  uint8_t lowerPattern;
  uint8_t latch;
  uint8_t xPosition;
};

struct TileInfo {
  uint8_t upperPattern;
  uint8_t lowerPattern;
  uint8_t attribute;
  int y;
  int x;
  bool horizontalFlip;
  bool background;
  uint8_t spriteIndex;
  bool priority;
  int bgTileIndex;
};
class PPU {
  enum class Mirroring { HORIZONTAL, VERTICAL } mirroringMode;
  bool renderFlag;
  uint8_t vram[2 * 1024];
  uint8_t internalBuffer;
  unsigned short baseAddress;
  bool addressLatch;
  bool inVblank;
  uint8_t registers[8];
  short scroll;
  unsigned short upperPattern, lowerPattern;
  bool bgTransparency[256][240];
  unsigned short attribute;
  unsigned short address;
  uint8_t xscroll;
  uint8_t yscroll;
  uint8_t OAM[256];
  std::vector<Sprite> secondaryOAM;
  int cyclesLeft;
  int cyclesNeeded;
  int currentScanline;
  std::vector<std::vector<RGB>> display =
      std::vector<std::vector<RGB>>(256, std::vector<RGB>(240));
  ; // take care of x and y
  Memory *memory;
  EdgeInterrupt *NMI;
  /*
  These are palletes used by the NES (taken verbatim from Blargg's palletes).
  Later we will add NTSC decoding to mirror what the NES actually does
   */

  constexpr static RGB palletes[64] = {
      {124, 124, 124}, {0, 0, 252},     {0, 0, 188},     {68, 40, 188},
      {148, 0, 132},   {168, 0, 32},    {168, 16, 0},    {136, 20, 0},
      {80, 48, 0},     {0, 120, 0},     {0, 104, 0},     {0, 88, 0},
      {0, 64, 88},     {0, 0, 0},       {0, 0, 0},       {0, 0, 0},
      {188, 188, 188}, {0, 120, 248},   {0, 88, 248},    {104, 68, 252},
      {216, 0, 204},   {228, 0, 88},    {248, 56, 0},    {228, 92, 16},
      {172, 124, 0},   {0, 184, 0},     {0, 168, 0},     {0, 168, 68},
      {0, 136, 136},   {0, 0, 0},       {0, 0, 0},       {0, 0, 0},
      {248, 248, 248}, {60, 188, 252},  {104, 136, 252}, {152, 120, 248},
      {248, 120, 248}, {248, 88, 152},  {248, 120, 88},  {252, 160, 68},
      {248, 184, 0},   {184, 248, 24},  {88, 216, 84},   {88, 248, 152},
      {0, 232, 216},   {120, 120, 120}, {0, 0, 0},       {0, 0, 0},
      {252, 252, 252}, {164, 228, 252}, {184, 184, 248}, {216, 184, 248},
      {248, 184, 248}, {248, 164, 192}, {240, 208, 176}, {252, 224, 168},
      {248, 216, 120}, {216, 248, 120}, {184, 248, 184}, {184, 248, 216},
      {0, 252, 252},   {248, 216, 248}, {0, 0, 0},       {0, 0, 0},
  };
  uint8_t programPalletes[32];

  coro::event &ppuExecutionStopped;
  coro::event &ppuCyclesAvailable;

  bool shouldQuit{false};

public:
  PPU(Memory *memory, EdgeInterrupt *NMI, coro::event &ppuExecutionStopped,
      coro::event &ppuCyclesAvailable);
  uint8_t readAddress(unsigned short address, bool external);
  void writeAddress(unsigned short address, uint8_t value);
  uint8_t getRegister(Registers reg);
  void setRegister(Registers reg, uint8_t value);
  uint8_t getIncrement();
  bool getSpriteMode();
  coro::task<void> consumeCycles(int cycles);
  void addCPUCycles(int cycles);
  void fetchTile(int tileNumber);
  TileInfo fetchSpriteTile(int oamIndex);
  void renderTile(TileInfo tileInfo);
  /*
  Read register is used when CPU reads something from PPU. It is
  an operation, unlike get register, which just gets the register without doing
  anything PPU specific
   */
  uint8_t readRegister(Registers reg);
  void writeRegister(Registers reg, uint8_t value);
  void writeOAM(uint8_t address, uint8_t value);
  unsigned short getNameTableAddress(uint8_t nameTableNumber);
  short getBasePatternTableAddress(bool background);
  bool shouldInterrupt();
  void cycle();
  RGB getPixel(int x, int y);
  std::vector<std::vector<RGB>> getFrame();
  void setPixel(int x, int y, RGB value);
  void clearTransparency();
  coro::task<void> generateFrame();
  void displayFrame();
  bool shouldRender();
  bool getCyclesLeft();
  void setMirroringMode(bool mode);
  unsigned short getAddress();
  bool canExecute();
  void raiseShouldQuit() { shouldQuit = true; }
};

/*
PPU has both RAM

 */

#endif