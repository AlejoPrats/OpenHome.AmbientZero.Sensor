#include "wifi_flow.hpp"
#include "wifi.hpp"
#include "pico/stdlib.h"
#include <cstring>

bool wifi_connect_flow(const DeviceConfig& cfg) {
    const char* ssid = (cfg.ssid[0] == '\0') ? "AmbientZero" : cfg.ssid;
    const char* password = (cfg.ssid[0] == '\0') ? "" : cfg.password;

    WiFi wifi(ssid, password);

    if (!wifi.connect()) {
        return false;
    }

    return true;
}
