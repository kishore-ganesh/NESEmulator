#include "apu.h"

APU::APU(){
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 4096;
    want.callback = NULL;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if(dev == 0){
        spdlog::error("Error opening Audio Device: {0:s}", SDL_GetError());
    }
    SDL_PauseAudioDevice(dev, 0);
    spdlog::info("Audio device: {0:d}", dev);
    spdlog::info("Having: Freq: {0:d}, Format: {1:d}, Samples: {2:d}", have.freq, have.format, have.samples);
}

unsigned char APU::readRegister(unsigned short address){
    spdlog::info("APU READ REGISTER: {0:x}", address);
    return 0;
}
void APU::writeRegister(unsigned short address, char value){
    static unsigned char k = 250;
    // k = k + 3;
    // printf("DEVICE: %d", dev);
    spdlog::info("Device is: {0:d}", dev);
    int status = SDL_QueueAudio(dev, &k, 1);
    spdlog::info("SPEAKER WRITE TEST: {0:d}", status);
    if(status == -1){
        spdlog::error("Error playing Audio Device: {0:s}", SDL_GetError());
    }
    
    
    spdlog::info("APU WRITE REGISTER: {0:x}, Value: {1:x}", address, value);
}