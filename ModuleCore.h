namespace ModuleCore
{
    using FunctionPtr = void (*)();
    void setup();
    void setupRealtimeLoop(FunctionPtr pRealtimeLoop);
    void loop();
} // namespace ModuleCore