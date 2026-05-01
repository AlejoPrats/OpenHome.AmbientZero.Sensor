#include "pico/stdlib.h"
#include "system/power_manager.hpp"
#include "domain/button_flow.hpp"
#include "app/app_main.hpp"
#include "drivers/rgb_led.hpp"
#include "portal/provisioning_flow.hpp"
#include "domain/hardware_init.hpp"
#include "services/scratch_handler.hpp"
#include "domain/ota_flow.hpp"
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
    pm.continue_deep_sleep(false);

    stdio_init_all();
    sleep_ms(3000);
    init_hardware();

    // ------------------------------------------------------------
    // 0. OTA override: check boot flag BEFORE any other flow
    // ------------------------------------------------------------
    BootFlag flag = get_boot_flag();

    if (flag == BootFlag::OTA_PENDING)
    {
        set_boot_flag(BootFlag::NONE);
        
        // Start OTA using your existing API
        ota_setup(NODE_IP); 

        while (true)
        {
            OtaResult result = ota_loop();

            if (result == OtaResult::ReadyToApply)
            {
                break;
            }

            if (result == OtaResult::Failed)
            {
                break;
            }

            sleep_ms(10);
        }
    }

    // ------------------------------------------------------------
    // 1. Normal boot logic continues here
    // ------------------------------------------------------------

    ButtonAction action = wait_for_button_action();

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

    pm.request_deep_sleep(DEEP_SLEEP_MS);
    pm.reboot_for_sleep();
}
