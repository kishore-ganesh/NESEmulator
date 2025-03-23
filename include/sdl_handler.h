#include "nes.h"
#include "util.h"
#include <SDL2/SDL.h>
#include <chrono>
#include <vector>
class SDLHandler {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  unsigned int frameStartTicks;
  unsigned int ticksInFrame;
  NES *nes;
  bool shouldQuit = false;
  std::chrono::steady_clock::time_point start, end;

public:
  SDLHandler(NES *nes) {
    SDL_Init(SDL_INIT_VIDEO);
    window =
        SDL_CreateWindow("NESEmulator", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 256, 240, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                SDL_TEXTUREACCESS_STREAMING, 256, 240);
    this->nes = nes;
    start = std::chrono::steady_clock::now();
    frameStartTicks = SDL_GetTicks();
    ticksInFrame = 18;
  }

  void handleEvent();
  void displayFrame(std::vector<std::vector<RGB>>);
  void begin();
};
