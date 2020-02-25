#include <SDL2/SDL.h>
#include "util.h"
#include "nes.h"
#include<vector>
class SDLHandler
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    NES* nes;
    bool shouldQuit = false;
    public:
    SDLHandler(NES* nes)
    {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("NESEmulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 240, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, 0);
        this->nes = nes;
    }

    void handleEvent();
    void displayFrame(std::vector<std::vector<RGB>>);
    void begin();
};
