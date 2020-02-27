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
            shouldQuit = true;
            break;
        }
        }
    };
}

void SDLHandler::displayFrame(std::vector<std::vector<RGB>> display)
{
    // printf("FRAME \n");
    unsigned char srcPixels[256*240*3];
    int index = 0;
    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            RGB pixel = display[x][y];
            // printf("INDEX: %d\n", index);
            srcPixels[index] = pixel.r;
            srcPixels[index+1] = pixel.g;
            srcPixels[index+2] = pixel.b;
            index+=3;

            // SDL_SetRenderDrawColor(renderer, pixel.r, pixel.g, pixel.b, 1);
            // SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    void* destPixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &destPixels, &pitch);
    memcpy(destPixels, srcPixels, 256*240*3);
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL ); //Update screen SDL_RenderPresent( gRenderer );
    SDL_RenderPresent(renderer);
    // SDL_RenderPresent(renderer);
}

void SDLHandler::begin(){
    while(!shouldQuit){
        handleEvent();
        nes->cycle();
        if(nes->shouldRender()){
            displayFrame(nes->getFrame());
        }
    }
}
