#pragma once

#include "config/device_config.hpp"
#include <string>

namespace WifiFlow
{

    /**
     * @brief Connects to WiFi using the provided device configuration.
     *
     * This function performs the full WiFi connection sequence:
     * - initializes the WiFi subsystem (if required)
     * - attempts connection using cfg.ssid and cfg.password
     * - waits for association and DHCP lease
     *
     * @param cfg  Device configuration containing WiFi credentials.
     *
     * @return true if the device successfully connected and obtained
     *         an IP address. Returns false on authentication failure,
     *         timeout, or any connection error.
     *
     * @note This function is synchronous at the flow level but relies
     *       on non‑blocking WiFi service operations internally.
     */
    bool wifi_connect_flow(const DeviceConfig &cfg);

    /**
     * @brief Starts WiFi Access Point mode and captive‑portal services.
     *
     * Enables AP mode using the given SSID and starts the supporting
     * network services required for provisioning:
     * - DHCP server
     * - DNS hijacking server
     * - HTTP server for the configuration portal
     *
     * @param ssid  SSID to broadcast in AP mode.
     *
     * @return true if AP mode and all services were started successfully.
     */
    bool start_access_point(const std::string &ssid);

    /**
     * @brief Performs a multi‑pass WiFi network scan.
     *
     * Executes several scan passes to improve reliability and gather
     * a more complete list of nearby networks. Results are stored
     * internally by the WiFi scan subsystem.
     *
     * @param passes  Number of scan iterations to perform.
     *
     * @note This function is non‑blocking per pass but must be called
     *       from a context that allows repeated polling of WiFi events.
     */
    void scan_networks(int passes);

}
