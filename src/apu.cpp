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
    status = 0;
}

unsigned char APU::readRegister(unsigned short address){
    spdlog::info("APU READ REGISTER: {0:x}", address);
    return 0;
}
void APU::writeRegister(unsigned short address, unsigned char value){
    // static unsigned char k = 250;
    // // k = k + 3;
    // // printf("DEVICE: %d", dev);
    // spdlog::info("Device is: {0:d}", dev);
    // int status = SDL_QueueAudio(dev, &k, 1);
    // spdlog::info("SPEAKER WRITE TEST: {0:d}", status);
    // if(status == -1){
    //     spdlog::error("Error playing Audio Device: {0:s}", SDL_GetError());
    // 
    // }  
    
    if(address >= 0x4000 && address <= 0x4003){
        //PULSE UNIT 1
        pulse1.writeRegister(address - 0x4000, value);
    }
    else if(address >= 0x4004 && address <= 0x4007){
        //PULSE UNIT 2
        pulse2.writeRegister(address - 0x4004, value);
    }
    else if(address >= 0x4008 && address <= 0x400B){
        //TRIANGLE WAVE UNIT
        triangle.writeRegister(address - 0x4008, value);
    }

    else if(address >= 0x400C && address <= 0X400f){
        //NOISE UNIT
    }
    else if(address >= 0x4010 && address <= 0x4013){

    }
    else if(address == 0x4015){
        status = value;
    }
    else if(address == 0x4017){
        frameCounter = value;
    }
    spdlog::info("APU WRITE REGISTER: {0:x}, Value: {1:x}", address, value);
}

void APU::addCPUCycles(char cpuCycles){
    cyclesLeft += cpuCycles / 2;
}

void APU::cycle(){
    unsigned char pulse1Output = pulse1.cycle();
    unsigned char pulse2Output = pulse2.cycle();
    unsigned char triangleOutput = 0, dmcOutput = 0, noiseOutput = 0;
    if((status & (int)EnableMasks::PULSE1)==0){
        pulse1Output = 0;
    }
    if((status & (int)EnableMasks::PULSE2)==0){
        pulse2Output = 0;
    }

    if((status & (int)EnableMasks::TRIANGLE)==0){
        triangleOutput = 0;
    }

    if((status & (int)EnableMasks::DMC)==0){
        dmcOutput = 0;
    }

    if((status & (int)EnableMasks::NOISE)==0){
        noiseOutput = 0;
    }

    unsigned short pulseOutput = 0, tndOutput = 0;
    if(!(pulse1Output == 0 && pulse2Output==0)){
        pulseOutput = 95.88/((8128)/(pulse1Output + pulse2Output) + 100);
    }

    if(!(triangleOutput == 0 && dmcOutput == 0 && noiseOutput == 0)){
        double tndDenominator = triangleOutput/8227  + (noiseOutput/12241) + (dmcOutput/22638);  

        tndOutput = 159.79/((1/(tndDenominator)) + 100);
    }

    unsigned char totalOutput = pulseOutput + tndOutput;
    SDL_QueueAudio(dev, &totalOutput, sizeof(totalOutput));

}

void PulseGenerator::writeRegister(unsigned short address, unsigned char value){
    spdlog::info("PULSE GENERATOR WRITE: {0:x}, value: {1:x}", address, value);
    switch(address){
        case 0 : {
            dutyCycle = (value >> 6);
            switch(dutyCycle){
                case 0: {currentSequence = 0x80; break;}
                case 1: {currentSequence = 0x60; break;}
                case 2: {currentSequence = 0x78; break;}
                case 3: {currentSequence = 0x9F; break;}
            }

            currentSequenceIndex = 0;
            lengthCounterHalt = (value >> 5) & 0x01;
            volumeFlag = (value >> 4) & 0x01;
            volume = (value & 0x0F);
            break;
        }

        case 1 : {
            //Sweep 
            break;
        }
        case 2 : {
            time&= 0xFF00;
            time|= value;
            timer = time;
            break;
        }
        case 3 : {
            time &= 0xFFF8;
            time |= (value & 0x07);
            timer = time;
            length = (value >> 3);
            break;
        }

    }
}

unsigned char PulseGenerator::cycle(){
    if(length == 0 || timer < 8){
        return 0;
    }

    bool currentSequenceBit = (currentSequence >> (7 - currentSequenceIndex)) & 0x01;

    if(timer==0){
        timer = time;
        currentSequenceIndex = (currentSequenceIndex + 1) % 8;
    }

    if(currentSequenceBit){
        // spdlog::info("VOLUME HIGH: {0:d}", volume);
        return volume;
    }
    
    
    
}

void TriangleGenerator::writeRegister(unsigned short address, unsigned char value){

}
