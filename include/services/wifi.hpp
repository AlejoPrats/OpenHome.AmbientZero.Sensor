#ifndef WIFI_HPP
#define WIFI_HPP

#include <string>
#include <stdint.h>
#include "pico/cyw43_arch.h"

/**
 * @brief Minimal WiFi STA (station mode) helper for the CYW43 chip.
 *
 * This class encapsulates the steps required to:
 *  - initialize the CYW43 driver
 *  - enable station mode
 *  - connect to an access point (open or WPA2)
 *
 * It stores the SSID and password internally and exposes a simple
 * connect() API used by the main application after provisioning.
 *
 * @note This class performs no scanning, AP mode, or captive‑portal
 *       logic. It is strictly for STA‑mode operation.
 */
class WiFi {
public:
    /**
     * @brief Constructs a WiFi object with the given credentials.
     *
     * @param ssid      Target network SSID.
     * @param password  Network password (empty for open networks).
     */
    WiFi(const std::string& ssid, const std::string& password);

    /**
     * @brief Initializes the CYW43 driver and enables station mode.
     *
     * Must be called before connect().
     *
     * @return true if initialization succeeded.
     */
    bool init();

    /**
     * @brief Attempts to connect to the configured WiFi network.
     *
     * Handles both open and WPA2 networks. This function blocks until
     * the connection succeeds or times out.
     *
     * @return true if the connection was established.
     */
    bool connect();

    /**
     * @brief Returns true if the device is currently connected.
     */
    bool is_connected() const;

private:
    std::string ssid;
    std::string password;
    bool connected = false;
};

#endif
