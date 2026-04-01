// sleeper.cpp
#include "drivers/sleeper.hpp"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

void Sleeper::sleepChunkMs(uint32_t ms) {
    watchdog_enable(ms, 1);

    while (true) {
        __wfi(); // never returns
    }
}