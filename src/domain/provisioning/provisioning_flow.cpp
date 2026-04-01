#include "portal/provisioning_flow.hpp"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/watchdog.h"

#include "domain/wifi_flow.hpp"
#include "config/device_config.hpp"
#include "drivers/rgb_led.hpp"
#include "network/wifi_scan.hpp"
#include "network/http_server.hpp"

extern volatile bool g_reboot_requested;

void provisioning_setup()
{
    stdio_init_all();
    sleep_ms(1500);

    DeviceConfig cfg{};
    load_config(cfg);

    if (!WifiFlow::startAccessPoint("AmbientZero-Setup"))
    {
        return;
    }

    WifiFlow::scanNetworks(5);

    if (!http.start())
    {
        return;
    }
}

void provisioning_loop()
{
    const uint32_t TIMEOUT_MS = 10 * 60 * 1000;
    absolute_time_t deadline = make_timeout_time_ms(TIMEOUT_MS);
    led.set_mode(LED_CAPTIVE);

    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0)
    {

        if (g_reboot_requested)
        {
            sleep_ms(10000);

            cyw43_arch_deinit();
            sleep_ms(100);

            watchdog_reboot(0, 0, 0);
            while (true)
            {
            }
        }

        // ⭐ NOTHING TO POLL
        // Your DHCP, DNS, HTTP, AP, and lwIP stack are all callback-driven.
        // They run in background threads or lwIP internal context.
        led.poll();
        sleep_ms(50); // keep the CPU alive without burning cycles
    }

    sleep_ms(500);

    cyw43_arch_deinit();
    watchdog_reboot(0, 0, 0);
}
