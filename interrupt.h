#ifndef INTERRUPT_INCLUDE
#define INTERRUPT_INCLUDE
struct EdgeInterrupt{
    bool status;
    bool prevStatus;
    EdgeInterrupt();
    void triggerInterrupt();
    void clearInterrupt();
    bool checkInterrupt();
};
#endif