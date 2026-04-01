#pragma once
#include <string>
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

/**
 * @brief Manages WiFi Access Point mode and its associated network interface.
 *
 * This class encapsulates the minimal logic required to start and stop
 * AP mode on the CYW43 WiFi chip. It exposes the resulting lwIP netif
 * and assigned IP address so higher‑level services (DHCP, DNS, HTTP)
 * can bind to the AP interface.
 *
 * @note This class does not run DHCP, DNS, or HTTP servers itself.
 *       It only brings up the AP interface.
 */
class AccessPoint
{
public:
    AccessPoint() = default;

    /**
     * @brief Starts WiFi Access Point mode with the given SSID.
     *
     * Initializes the WiFi subsystem (if required), configures AP mode,
     * assigns a static IP address, and registers the AP network interface
     * with lwIP.
     *
     * @param ssid  SSID to broadcast.
     *
     * @return true if AP mode was successfully started.
     */

    bool start(const std::string &ssid);

    /**
     * @brief Stops AP mode and removes the AP network interface.
     *
     * Shuts down the AP interface and returns the WiFi subsystem to an
     * uninitialized or idle state.
     */
    void stop();
    
    /**
     * @brief Returns the lwIP network interface associated with AP mode.
     *
     * @return Pointer to the AP netif, or nullptr if AP mode is inactive.
     */
    struct netif *getNetif() const { return apNetif; }

    /**
     * @brief Returns the static IP address assigned to the AP interface.
     */
    ip4_addr_t getIp() const { return apIp; }

private:
    struct netif *apNetif = nullptr;
    ip4_addr_t apIp{};
};
