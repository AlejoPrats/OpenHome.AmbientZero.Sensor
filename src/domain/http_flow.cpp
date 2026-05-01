#include "domain/http_flow.hpp"
#include "services/http_client.hpp"
#include "protocol/json_parser.hpp"
#include "protocol/server_response.hpp"
#include "config/device_config.hpp"
#include "services/scratch_handler.hpp"
#include "pico/stdlib.h"
#include <cstdio>
#include <cstring>
#include <string>

bool send_measurement_flow(
    float temperature,
    float humidity,
    uint16_t battery_raw,
    bool isSignaling,
    RGBLed &led,
    const DeviceConfig &cfg)
{
    // ---------------------------------------------------------
    // Build JSON body
    // ---------------------------------------------------------
    char deviceIdValue[80];

    if (cfg.id[0])
    {
        snprintf(deviceIdValue, sizeof(deviceIdValue), "\"%s\"", cfg.id);
    }
    else
    {
        snprintf(deviceIdValue, sizeof(deviceIdValue), "null");
    }

    char jsonBody[256];
    snprintf(jsonBody, sizeof(jsonBody),
             "{"
             "\"temperature\": %.2f,"
             "\"deviceId\": %s,"
             "\"adcReading\": %d,"
             "\"isSignaling\": %s,"
             "\"Version\": \"%s\""
             "}",
             temperature,
             deviceIdValue,
             battery_raw,
             isSignaling ? "true" : "false",
             cfg.version);

    // ---------------------------------------------------------
    // Perform HTTP request using unified client
    // ---------------------------------------------------------
    HttpJsonRequest req;
    req.ip = NODE_IP;
    req.port = HTTP_API_PORT;
    req.path = HTTP_API_PATH;

    req.verb = "PUT";
    req.body = jsonBody;

    HttpClient client;

    std::string response = client.request(req);
    if (response.empty())
    {
        led.set_mode_blocking(LED_ERROR);
        return false;
    }

    // ---------------------------------------------------------
    // Parse server response
    // ---------------------------------------------------------
    ServerResponse serverResponse{};

    if (!parse_server_response(response.c_str(), serverResponse))
    {
        led.set_mode_blocking(LED_ERROR);
        return false;
    }

    // ---------------------------------------------------------
    // Update device ID if server assigned one
    // ---------------------------------------------------------
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

    // ---------------------------------------------------------
    // LED feedback
    // ---------------------------------------------------------
    if (serverResponse.isLightEnabled)
    {
        led.set_mode_blocking(LED_OK);
    }

    if (serverResponse.shouldUpdate)
    {
        set_boot_flag(BootFlag::OTA_PENDING);
    }

    return true;
}
