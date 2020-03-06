#include "controller.h"
void Controller::capture(){
    spdlog::info("Capturing Input");
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    for(KEYMAP::iterator key = keyMap.begin(); key!= keyMap.end(); key++){
        if(keys[key->first]){
            spdlog::info("KEY PRESSED: {0:d}", (unsigned char)key->second);
            capturingStatus |= (unsigned char)key->second;
        }
    }
    spdlog::info("Capturing Status: {0:d}", capturingStatus);

}

void Controller::stopCapture(){
    spdlog::info("Stopped capturing input");
    providingStatus = capturingStatus;
    capturingStatus = 0;
}

// void Controller::printStatus(){
//     spdlog::info(" 
//         A: {0:b}\n \ 
//         B: {0:b}\n \
//     ")
// }

bool Controller::readNext(){
    spdlog::info("Providing status: {}", providingStatus);
    bool lastBit = providingStatus & 0x01;
    spdlog::info("Last bit: {}", lastBit);
    providingStatus >>= 1;
    return lastBit;
}