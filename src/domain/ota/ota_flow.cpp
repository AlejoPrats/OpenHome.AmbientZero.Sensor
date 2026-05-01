#include "domain/ota_flow.hpp"
#include "config/device_config.hpp"
#include "domain/wifi_flow.hpp"
#include "services/ota_service.hpp"
#include "system/power_manager.hpp"
#include "pico/stdlib.h"

static OtaService g_ota;

void ota_setup(const char *server_ip)
{
    DeviceConfig cfg{};
    bool hasConfig = load_config(cfg);

    if (!WifiFlow::wifi_connect_flow(cfg))
    {
        return;
    }

    HttpClient client;
    PowerManager pm;
    bool isOtaRunning = false;
    while (true)
    {
        if (isOtaRunning)
        {
            OtaResult r = ota_loop();

            if (r == OtaResult::ReadyToApply)
            {
                client.close();
                sleep_ms(50);

                pm.enter_flash_safe_state(true);

                // Jump into RAM and never return
                FlashWriter::apply_ota_to_main(ota_get_firmware_size());
            }

            if (r == OtaResult::Failed)
            {
                isOtaRunning = false;
            }
        }
        else
        {
            g_ota.start();
            isOtaRunning = true;
        }
    }
}

OtaResult ota_loop()
{
    g_ota.poll();

    if (!g_ota.is_finished())
    {
        return OtaResult::Running;
    }

    if (!g_ota.was_successful())
    {
        return OtaResult::Failed;
    }

    return OtaResult::ReadyToApply;
}

uint32_t ota_get_firmware_size()
{
    return g_ota.getFirmwareSize();
}
