#ifndef CONTROLLER_INCLUDE
#define CONTROLLER_INCLUDE
#include<SDL2/SDL.h>
#include<map>
#include "util.h"
class Controller{

    unsigned char capturingStatus;
    unsigned char providingStatus;
    enum class ButtonMasks{
        A = 1,
        B = 2,
        SELECT = 4,
        START = 8,
        UP = 16,
        DOWN = 32,
        LEFT = 64,
        RIGHT = 128
    };
    using KEYMAP = std::map<SDL_Scancode, ButtonMasks>;
    KEYMAP keyMap;
    public:
    Controller(){
        keyMap = {
            {SDL_SCANCODE_A, ButtonMasks::A},
            {SDL_SCANCODE_B, ButtonMasks::B},
            {SDL_SCANCODE_RSHIFT, ButtonMasks::SELECT},
            {SDL_SCANCODE_UP, ButtonMasks::UP},
            {SDL_SCANCODE_DOWN, ButtonMasks::DOWN},
            {SDL_SCANCODE_LEFT, ButtonMasks::LEFT},
            {SDL_SCANCODE_RIGHT, ButtonMasks::RIGHT},
            {SDL_SCANCODE_KP_ENTER, ButtonMasks::START},
        };
    }
    void capture();

    void stopCapture();

    bool readNext();
};
#endif