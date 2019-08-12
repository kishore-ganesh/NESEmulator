struct EdgeInterrupt{
    bool status;
    bool prevStatus;
    EdgeInterrupt();
    void triggerInterrupt();
    void clearInterrupt();
    bool checkInterrupt();
};
