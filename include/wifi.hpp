#ifndef WIFI_HPP
#define WIFI_HPP

#include <string>
#include <stdint.h>
#include "pico/cyw43_arch.h"


class WiFi {
public:
    WiFi(const std::string& ssid, const std::string& password);

    bool init();        // Initialize CYW43 and enable STA mode
    bool connect();     // Connect to WiFi (open or WPA2)
    bool is_connected() const;

private:
    std::string ssid;
    std::string password;
    bool connected = false;
};

#endif
