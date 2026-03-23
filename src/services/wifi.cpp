#include "wifi.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

WiFi::WiFi(const std::string& ssid, const std::string& password)
    : ssid(ssid), password(password) {}

bool WiFi::init() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE)) {
        return false;
    }

    cyw43_arch_enable_sta_mode();
    return true;
}

bool WiFi::connect() {
    if (!init()) {
        return false;
    }

    // Determine authentication type
    uint32_t auth = CYW43_AUTH_WPA2_AES_PSK;

    if (password.empty()) {
        auth = CYW43_AUTH_OPEN;
    }

    int result = cyw43_arch_wifi_connect_timeout_ms(
        ssid.c_str(),
        password.empty() ? nullptr : password.c_str(),
        auth,
        10000 // 10 second timeout
    );

    connected = (result == 0);
    return connected;
}

bool WiFi::is_connected() const {
    return connected;
}
