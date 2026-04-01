#include "pico/stdlib.h"
#include "system/power_manager.hpp"
#include "domain/button_flow.hpp"
#include "app/app_main.hpp"
#include "drivers/rgb_led.hpp"
#include "portal/provisioning_flow.hpp"
#include "domain/hardware_init.hpp"
#include <cstdio>

ButtonAction wait_for_button_action()
{
    if (button_status() == ButtonStatus::PRESSED)
    {
        while (true)
        {
            ButtonAction action = button_poll();
            led.poll();

            if (action != ButtonAction::NONE)
            {
                return action;
            }

            sleep_ms(5);
        }
    }
    else
    {
        return ButtonAction::NORMAL_BOOT;
    }
}

int main()
{
    PowerManager pm;
    pm.continueDeepSleep(false);

    stdio_init_all();
    sleep_ms(3000);
    init_hardware();

    // 1. Classify user intent (non-blocking, with LED feedback)
    ButtonAction action = wait_for_button_action();

    // 2. Decide which mode to run
    switch (action)
    {
    case ButtonAction::SIGNAL_PRESS:
        app_main_run_cycle(true);
        break;

    case ButtonAction::PROVISION_PRESS:
        app_main_start_provisioning();
        provisioning_loop();
        break;

    case ButtonAction::NORMAL_BOOT:
    default:
        app_main_run_cycle(false);
        break;
    }

    // 3. Normal cycle → deep sleep
    pm.requestDeepSleep(1200000);
    pm.rebootForSleep();
}
