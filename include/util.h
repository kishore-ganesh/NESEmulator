#ifndef UTIL_INCLUDE
#define UTIL_INCLUDE

#include "spdlog/spdlog.h"
struct RGB{
    unsigned char r, g, b;
    bool operator == (RGB other){
        return this->r==other.r  && this->g == other.g && this->b == other.b;
    }
};

#endif
