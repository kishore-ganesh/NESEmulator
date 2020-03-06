#include "controller.h"
void Controller::capture(){
    SPDLOG_INFO("Capturing Input");
    capturingStatus = 0;
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    for(KEYMAP::iterator key = keyMap.begin(); key!= keyMap.end(); key++){
        if(keys[key->first]){
            // SPDLOG_INFO("KEY PRESSED: {0:d}", (unsigned char)key->second);
            SPDLOG_INFO("Key pressed: {0:s}", debugMap[key->first]);
            capturingStatus |= (unsigned char)key->second;
        }
    }
    SPDLOG_INFO("Capturing Status: {0:d}", capturingStatus);

}

void Controller::stopCapture(){
    SPDLOG_INFO("Stopped capturing input");
    providingStatus = capturingStatus;
    
}

// void Controller::printStatus(){
//     SPDLOG_INFO(" 
//         A: {0:b}\n \ 
//         B: {0:b}\n \
//     ")
// }

bool Controller::readNext(){
    SPDLOG_INFO("Providing status: {}", providingStatus);
    bool lastBit = providingStatus & 0x01;
    SPDLOG_INFO("Last bit: {}", lastBit);
    providingStatus >>= 1;
    return lastBit;
}