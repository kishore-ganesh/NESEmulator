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
            // if(y==239 && x==255)
            // spdlog::info("Index is: {0:d}", index);
            // SDL_SetRenderDrawColor(renderer, pixel.r, pixel.g, pixel.b, 1);
            // SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    //Check this
    void* destPixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &destPixels, &pitch);
    // spdlog::info("Pitch is : {0:d}", pitch);
    memcpy(destPixels, srcPixels, 256*240*3);
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL ); //Update screen SDL_RenderPresent( gRenderer );
    SDL_RenderPresent(renderer);
    // SDL_RenderPresent(renderer);
}

void SDLHandler::begin(){
    while(!shouldQuit){
        handleEvent();
        SPDLOG_INFO("CYCLE");
        nes->cpuCycle();//Refactor into nes->CPucycle and nes->ppucycle so that we can get ppu to run as long
        // nes->cycle();
        // if(nes->shouldRender()){
        //     displayFrame(nes->getFrame());
        // }
        // nes->apu->writeRegister(0, 0);
        while(nes->ppuCyclesLeft()){
            SPDLOG_INFO("IN PPU CYCLE");
            nes->ppuCycle();
            if(nes->shouldRender()){
                // spdlog::info("SHOULD RENDER");
                displayFrame(nes->getFrame());
            }
        }
        
    }
}
