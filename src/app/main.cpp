#include "pico/stdlib.h"
#include "power_manager.hpp"
#include "app_main.hpp"
#include <cstdio>

int main()
{
    PowerManager pm;
    pm.continueDeepSleep(false);
    
    stdio_init_all();
    sleep_ms(3000);

    // --- Normal application cycle ---
    app_main();

    // --- Prepare for next cycle ---
    pm.requestDeepSleep(1200000);

    pm.rebootForSleep();
    return 0; // never reached
}
