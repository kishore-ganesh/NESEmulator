#include <SDL2/SDL.h>
#include "util.h"
#include<vector>
class SDLHandler
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    public:
    SDLHandler()
    {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("NESEmulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 240, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    void handleEvent();
    void displayFrame(std::vector<std::vector<RGB>>);
};
