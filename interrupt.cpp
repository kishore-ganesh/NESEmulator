#include "interrupt.h"

EdgeInterrupt::EdgeInterrupt(){
    clearInterrupt();
}
void EdgeInterrupt::triggerInterrupt(){
    prevStatus = false;
    status = true;
}

void EdgeInterrupt::clearInterrupt(){
    prevStatus = true;
    status = true; // check this
}

bool EdgeInterrupt::checkInterrupt(){
    if(!status&&prevStatus){
        prevStatus = false;
        return true;
    }
    return false;
}