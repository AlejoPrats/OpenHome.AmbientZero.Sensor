#pragma once

#include <cstdint>
#include "sleeper.hpp"

class PowerManager
{
public:
    void continueDeepSleep(bool isWifiInitialized);        // called at start of main
    void requestDeepSleep(uint64_t ms); // called at end of flow
    void rebootForSleep();

private:
    void prepareWifiForSleep();
    void switchToROSC();          // your existing, working implementation
    void reduceClocksAfterROSC(); // new
    Sleeper sleeper;
    void enterLowPower(bool isWifiInitialized);
};
