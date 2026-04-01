#include "domain/http_flow.hpp"
#include "services/http_client.hpp"
#include "protocol/json_parser.hpp"
#include "protocol/server_response.hpp"
#include "config/device_config.hpp"
#include "pico/stdlib.h"
#include <cstdio>
#include <cstring>

bool send_measurement_flow(
    float temperature,
    float humidity,
    uint16_t battery_raw,
    bool isSignaling,
    RGBLed &led,
    const DeviceConfig &cfg)
{
    char deviceIdValue[80];

    if (cfg.id[0])
    {
        // GUID exists → wrap in quotes
        snprintf(deviceIdValue, sizeof(deviceIdValue), "\"%s\"", cfg.id);
    }
    else
    {
        // No GUID → literal null
        snprintf(deviceIdValue, sizeof(deviceIdValue), "null");
    }

    char jsonBody[256];
    snprintf(jsonBody, sizeof(jsonBody),
             "{"
             "\"temperature\": %.2f,"
             "\"deviceId\": %s,"
             "\"adcReading\": %d,"
             "\"isSignaling\": %s"
             "}",
             temperature,
             deviceIdValue,
             battery_raw,
             isSignaling ? "true" : "false");

    bool done = false;
    bool success = false;

    http_request(
        HTTP_API_IP,
        HTTP_API_PORT,
        "PUT",
        "/AmbientTemperature",
        jsonBody,
        [&](const std::string &response)
        {
            ServerResponse serverResponse{};
            if (parse_server_response(response.c_str(), serverResponse))
            {
                success = true;

                // Only update config if server sent a real ID
                if (serverResponse.hasDeviceId && cfg.id[0] == '\0')
                {
                    DeviceConfig newCfg = cfg;

                    size_t copyLen = strlen(serverResponse.deviceId);
                    if (copyLen >= sizeof(newCfg.id))
                        copyLen = sizeof(newCfg.id) - 1;

                    memcpy(newCfg.id, serverResponse.deviceId, copyLen);
                    newCfg.id[copyLen] = '\0';

                    save_config(newCfg);
                }

                if (serverResponse.isLightEnabled)
                {
                    led.set_mode_blocking(LED_OK);
                }
            }
            else
            {
                led.set_mode_blocking(LED_ERROR);
            }

            done = true;
        });

    while (!done)
    {
        http_poll();
        sleep_ms(10);
    }

    return success;
}
