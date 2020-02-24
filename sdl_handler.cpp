#include "sdl_handler.h"

void SDLHandler::handleEvent()
{
    SDL_Event event;
    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        {
            SDL_Quit();
            break;
        }
        }
    };
}

void SDLHandler::displayFrame(std::vector<std::vector<RGB>> display)
{
    for (int x = 0; x < 256; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            RGB pixel = display[x][y];
            SDL_SetRenderDrawColor(renderer, pixel.r, pixel.g, pixel.b, 1);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_RenderPresent(renderer);
}
