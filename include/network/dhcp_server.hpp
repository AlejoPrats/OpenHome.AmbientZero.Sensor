#pragma once
#include "lwip/netif.h"

/**
 * @brief Minimal DHCP server bound to a specific lwIP network interface.
 *
 * This class provides a lightweight DHCP server implementation used
 * during provisioning when the device operates in Access Point mode.
 * It listens for DHCPDISCOVER/DHCPREQUEST packets and responds with
 * a fixed IP lease suitable for captive‑portal operation.
 *
 * @note This server is intentionally minimal. It supports only the
 *       subset of DHCP required for a single‑client AP environment.
 */
class DhcpServer
{
public:
    DhcpServer() = default;

    /**
     * @brief Starts the DHCP server on the given AP network interface.
     *
     * Binds a UDP PCB to port 67 and begins responding to DHCP requests
     * originating from clients connected to the AP interface.
     *
     * @param apNetif  The lwIP netif representing the AP interface.
     *
     * @return true if the server was successfully started.
     */
    bool start(struct netif *apNetif);

    /**
     * @brief Stops the DHCP server and releases its UDP PCB.
     *
     * After calling stop(), no further DHCP packets will be processed.
     */
    void stop();

private:
    /**
     * @brief Callback invoked by lwIP when a DHCP packet is received.
     *
     * Parses the incoming request and sends the appropriate DHCP reply.
     * This function is registered internally when the server starts.
     */
    static void recvCallback(void *arg, struct udp_pcb *pcb,
                             struct pbuf *p, const ip_addr_t *addr, u16_t port);

    struct udp_pcb *pcb = nullptr;
};
