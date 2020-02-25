#include "nes.h"
#include "sdl_handler.h"
int main(int argc, char* argv[]){
    NES nes(argv[1]);
    SDLHandler handler(&nes);
    handler.begin();
    // while(true){
    //     handler.handleEvent();
    //     nes.cycle();
    //     if(nes.shouldRender()){
    //         handler.displayFrame(nes.getFrame());
    //     }
    // }
    // nes.start();
}