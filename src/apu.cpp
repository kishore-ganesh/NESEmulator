#include "apu.h"
#include<math.h>

APU::APU(){
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16LSB;
    want.channels = 1;
    want.samples = 2048;
    want.callback = NULL;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if(dev == 0){
        spdlog::error("Error opening Audio Device: {0:s}", SDL_GetError());
    }
    SDL_PauseAudioDevice(dev, 0);
    spdlog::info("Audio device: {0:d}", dev);
    spdlog::info("Having: Freq: {0:d},  Format: {1:d}, Samples: {2:d}, Channels: {3:d},", have.freq, have.format, have.samples, have.channels);
    std::fill(samples, samples + numSamples, 0);
    std::fill(samplesToProcess, samplesToProcess + numSamples*freqScalingFactor, 0);
    samplesIndex = 0;
    status = 0;
    cyclesLeft = 0;
    currentCycle = 0;
}

unsigned char APU::readRegister(unsigned short address){
    // spdlog::info("APU READ REGISTER: {0:x}", address);
    return 0;
}
void APU::writeRegister(unsigned short address, unsigned char value){
    
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
    // spdlog::info("APU WRITE REGISTER: {0:x}, Value: {1:x}", address, value);
}

void APU::addCPUCycles(char cpuCycles){
    cyclesLeft += cpuCycles / 2;
    // cyclesLeft = 8192;
}

bool APU::getCyclesLeft(){
    // spdlog::info("GET CYCLES LEFT");
    return cyclesLeft > 0;
}

void APU::cycle(){
    if(cyclesLeft > 0){
        cyclesLeft--;
        currentCycle++;
    }
    else{
        return;
    }
    bool fiveStep = frameCounter & 0x80;
    switch(currentCycle){
        case 3729: {
            triangle.linear();
            break;
        }
        case 7457:{
            pulse1.sweep();
            pulse2.sweep();
            triangle.lengthCounter();
            triangle.linear();
            break;
        }
        case 11186: {
            triangle.linear();
            break;
        }
        case 14914: {
            break;
        }
        case 14915: {
            break;
        }
        case 14916: {

            if(!fiveStep){
                currentCycle = 0;
                pulse1.sweep();
                pulse2.sweep();
                triangle.lengthCounter();
                triangle.linear();
            }
        }
        case 18641: {

        }
        
        case 18642: {
            if(fiveStep){
                pulse1.sweep();
                pulse2.sweep();
                triangle.lengthCounter();
                triangle.linear();
                currentCycle = 0;
            }
        }
    }
    

    // spdlog::info("PULSE CYCLES FETCHED");
    unsigned short pulse1Output = 0, pulse2Output = 0, triangleOutput = 0, dmcOutput = 0, noiseOutput = 0;
    pulse1Output = pulse1.cycle();
    pulse2Output = pulse2.cycle();
    triangle.cycle();
    triangleOutput = triangle.cycle();
    // spdlog::info("Triangle output: {0:d}", triangleOutput);
    if((status & (int)EnableMasks::PULSE1)==0){
        // spdlog::info("PULSE 1 DISABLED");
        pulse1Output = 0;
    }
    if((status & (int)EnableMasks::PULSE2)==0){
        // spdlog::info("PULSE 2 DISABLED");
        pulse2Output = 0;
    }

    if((status & (int)EnableMasks::TRIANGLE)==0){
        // spdlog::info("TRIANGLE DISABLED");
        triangleOutput = 0;
    }

    if((status & (int)EnableMasks::DMC)==0){
        // spdlog::info("DMC DISABLED");
        dmcOutput = 0;
    }

    if((status & (int)EnableMasks::NOISE)==0){
        // spdlog::info("NOISE DISABLED");
        noiseOutput = 0;
    }

    double pulseOutput = 0, tndOutput = 0;
    // spdlog::info("PULSE 1 ENABLE  is: {0:b}, PULSE 2 ENABLE: {1:b}", )
    // spdlog::info("PULSE 1 output is: {0:d}, PULSE 2 output is: {1:d}", pulse1Output, pulse2Output);
    if(!(pulse1Output == 0 && pulse2Output==0)){
        pulseOutput = 95.88/((8128.0)/(pulse1Output + pulse2Output) + 100.0);
    }

    // spdlog::info("PULSE OUTPUT IS: {0:f}", pulseOutput);

    if(!(triangleOutput == 0 && dmcOutput == 0 && noiseOutput == 0)){

        double tndDenominator = triangleOutput/8227.0 + (noiseOutput/12241.0) + (dmcOutput/22638.0);  

        tndOutput = 159.79/((1.0/(tndDenominator)) + 100);
    }

    // double totalOutput = pulseOutput + tndOutput;
    double totalOutput = pulseOutput;
    // double totalOutput = pulse1Output;
    // double totalOutput = sin(2*3.14*10*samplesIndex);
    // int sign = (samples
    // Index%8==0)?1:-1;
    // double rescaledOutput = ((pulse1Output+pulse2Output)/30.0)*(65536/2);
    double rescaledOutput = totalOutput * 32768;
    // spdlog::info("Queued: {0:d}SDL_GetQueuedAudioSize()
    unsigned short total8BitOutput = rescaledOutput;
    // total8BitOutput = (sin(samplesIndex)+1.0)*100;
    // if(totalOutput > 0)
    int sample = total8BitOutput;
    samplesToProcess[samplesIndex] = sample;

    // if((currentCycle %19)==0){
    //     // samples[samplesIndex] = total8BitOutput;
    //     samples[samplesIndex] = sample/19;
    //     sample = 0;
    // }

    constexpr auto lanczosLobes = 3;

    auto sinc = [](float t) {
      if(t==0.0) return 1.0;
      return sin(M_PI * t) / (M_PI*t);
    };

    auto lanczos = [&sinc, lanczosLobes](float t) {
      if(t < -lanczosLobes) {
        return 0.0;
      }
      if(t > lanczosLobes) {
        return 0.0;
      }
      return sinc(t) * sinc(t / lanczosLobes);
    };

    constexpr int averageSamples = 0;

    if(samplesIndex == (numSamples * freqScalingFactor - 1)){
        auto hamming = [](float freq, int i, int M) {
          if(i == M/2) {
            return  2 * M_PI * freq;
          } 

          auto t2 = 0.42 - 0.5 * cos(2 * M_PI * i / M)  + 0.08 * cos(4 * M_PI * i / M);

          auto t1 = sin(2 * M_PI * freq * (i - M / 2)) / (i - M/2);
          return t1 * t2;
        };
    
        // spdlog::info("Playing");
        constexpr auto freqFactor = 20;
        for(int s = 0; s < numSamples; s++) {
          float downsampledSample = 0;
          auto sumLanczos = 0.0;
          for(int a = -freqFactor*lanczosLobes; a < freqFactor*lanczosLobes; a++){
              if(!averageSamples) {          
                auto newS = (s + 0.5) * freqFactor - 0.5;
                size_t newIndex = floor(newS) + a;
                if(newIndex >= 0 && newIndex < samplesIndex) {
                  downsampledSample += samplesToProcess[newIndex] * lanczos( (a - newS + floor(newS)) / (double)freqFactor);
                  sumLanczos += lanczos( (a - newS + floor(newS)) / (double)freqFactor);
                }
              }
              else{
                downsampledSample += samplesToProcess[s + a];
              }
            }

          // for(int a = 0; a < 100; a++) {
          //   if( (s -a) >=0 ) {
          //     downsampledSample += samplesToProcess[s - a] * hamming(0.02, a, 100);
          //   }
          // }
       
          // if(s % freqScalingFactor == 0) {
            // spdlog::error("Sample is: {}", downsampledSample);
            if(!averageSamples) {
              samples[s] = (downsampledSample / sumLanczos) * 8.0;
            // }
            // else{
            //   // samples[s / freqScalingFactor] = downsampledSample / (freqScalingFactor * lanczosLobes * 2);
            // }
            
          }
        }

        samplesIndex = 0;
        int status = SDL_QueueAudio(dev, samples, 2048*sizeof(total8BitOutput));
        if(status == -1){
            spdlog::error("Unable to play: {0:s}", SDL_GetError());
        }
        memset(samples, 0, 2048*2);
    }
    else{
        samplesIndex++;
    }
    

}

void PulseGenerator::sweep(){
    //During half period only
    if(currentSweepPeriod == 0){
        currentSweepPeriod = sweepPeriod;
        if(sweepUnit & 0x80){
            bool negate = (sweepUnit >> 3) & 0x01;
            unsigned char shift = (sweepUnit) & 0x07;
            unsigned short change = time >> shift;
            if(negate){
                change = -change -1;
            }
            time+=change;

            //mutehere
        }
    }
    else{
        currentSweepPeriod = currentSweepPeriod - 1;
    }
    

}

void PulseGenerator::writeRegister(unsigned short address, unsigned char value){
    // spdlog::info("PULSE GENERATOR WRITE: {0:x}, value: {1:x}", address, value);
    switch(address){
        case 0 : {
            dutyCycle = (value >> 6);
            switch(dutyCycle){
                case 0: {currentSequence = 0x80; break;}
                case 1: {currentSequence = 0x60; break;}
                case 2: {currentSequence = 0x78; break;}
                case 3: {currentSequence = 0x9F; break;}
            }

            lengthCounterHalt = (value >> 5) & 0x01;
            volumeFlag = (value >> 4) & 0x01;
            // spdlog::info("Volume flag is: {0:b}", volumeFlag);
            volume = (value & 0x0F);
            break;
        }

        case 1 : {
            //Sweep 
            sweepUnit = value;
            sweepPeriod = (sweepUnit >> 4) & 0x07;
            currentSweepPeriod = sweepPeriod;
            break;
        }
        case 2 : {
            time&= 0x0F00;
            time|= value;
            timer = time;
            break;
        }
        case 3 : {

            time &= 0x00FF;
            time |= ((value & 0x07)<<8);
            timer = time;
            currentSequenceIndex = 0;
            length = (value >> 3);
            break;
        }

    }
}

unsigned short PulseGenerator::cycle(){
    if(length == 0 || time < 8){
        return 0;
    }
    timer--;
    bool currentSequenceBit = (currentSequence >> (7 - currentSequenceIndex)) & 0x01;

    if(timer==0){
        timer = time;
        // spdlog::info("Cur rent sequence bit: {0:b}", currentSequenceBit);
        currentSequenceIndex = (currentSequenceIndex + 1) % 8;
    }

    
    // spdlog::info("Current duty cycle: {0:d}", dutyCycle);
    //  spdlog::info("Current sequence: {0:d}", currentSequence);;
    if(currentSequenceBit){
        // spdlog::info("VOLUME HIGH: {0:d}", volume);
        return volume;
    }

    
    
    return 0;
    
    
    
}

void TriangleGenerator::writeRegister(unsigned short address, unsigned char value){
    // spdlog::info("Triangle write to {:x} value {:x}", address, value);
    switch(address){
        case 0x0: {
            controlFlag = value & 0x80;
            reloadValue = value & 0x7F;
            break;
        }
        case 0x2: {
            time&= 0x0F00;
            time|= value;
            timer = time;
            break;
        }

        case 0x3:{
            // spdlog::info("Length");
            time &= 0x00FF;
            time |= ((value & 0x07)<<8);
            timer = time;
            currentSequenceIndex = 0;
            length = (value >> 3);
            linearCounterReload = true;
            break;
        }
    }
}

void TriangleGenerator::linear(){
    if(linearCounterReload){
        linearCounter = reloadValue;
    }
    else{
        if(linearCounter > 0){
            linearCounter --;
        }
        linearCounterReload = controlFlag;
    }
}

void TriangleGenerator::lengthCounter(){
    if(!controlFlag){
        if(length > 0){
            length --;
        }
    }
}

unsigned short TriangleGenerator::cycle(){
    // spdlog::error("Time value is: {}", time);
    if(length == 0 || time < 2 || linearCounter==0){
        return 0;
    }
    // spdlog::info("Time is: {:d}", time);
    timer--;

    if(timer==0){
        timer = time;
        // spdlog::info("Cur rent sequence bit: {0:b}", currentSequenceBit);
        currentSequenceIndex = (currentSequenceIndex + 1) % 32;
    }

    
    // spdlog::info("Current duty cycle: {0:d}", dutyCycle);
    //  spdlog::info("Current sequence: {0:d}", currentSequence);;
    return sequence[currentSequenceIndex];

}