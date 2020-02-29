#include "nes.h"
#include "sdl_handler.h"

void handleLoggingLevel(char* arg){
    
    if(strcmp("-debug", arg)==0){
        spdlog::info("Logging Mode: Debug");
        spdlog::set_level(spdlog::level::debug);
    }
    else if(strcmp("-error", arg)==0){
        spdlog::info("Logging Mode: Error");
        spdlog::set_level(spdlog::level::err);
    }
    else if(strcmp("-warn", arg)==0){
        spdlog::info("Logging Mode: Warn");
        spdlog::set_level(spdlog::level::warn);
    }
    else if(strcmp("-critical", arg)==0){
        spdlog::info("Logging Mode: Critical");
        spdlog::set_level(spdlog::level::critical);
    }
    else if(strcmp("-info", arg)==0){
        spdlog::info("Logging Mode: Info");
        spdlog::set_level(spdlog::level::info);
    }
}
int main(int argc, char* argv[]){
    
    if(argc>2){
        handleLoggingLevel(argv[2]);
    }
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