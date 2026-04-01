#pragma once
#include <lwip/ip4_addr.h>
#include <lwip/udp.h>

/**
 * @brief Minimal DNS hijacking server for captive‑portal operation.
 *
 * This server listens for DNS queries on UDP port 53 and responds to
 * *all* queries with the Access Point’s IP address. This forces client
 * devices to route all domain lookups to the device, enabling reliable
 * captive‑portal detection across platforms.
 *
 * @note This is intentionally minimal and supports only the subset of
 *       DNS required for captive‑portal redirection. It does not
 *       implement recursion, caching, or multi‑record responses.
 */
class DnsServer
{
public:
    /**
     * @brief Starts the DNS server bound to the AP IP address.
     *
     * Creates a UDP PCB, binds it to port 53, and begins responding to
     * incoming DNS queries with a fixed A record pointing to `apIp`.
     *
     * @param apIp  The IP address of the Access Point interface.
     *
     * @return true if the server was successfully started.
     */
    bool start(ip4_addr_t apIp);

    /**
     * @brief Stops the DNS server and releases its UDP PCB.
     */
    void stop();

private:
    /**
     * @brief lwIP callback invoked when a DNS packet is received.
     *
     * Dispatches the packet to handleQuery() and sends the appropriate
     * DNS response.
     */
    static void recvCallback(void *arg, udp_pcb *pcb,
                             pbuf *p, const ip_addr_t *addr, u16_t port);

    /**
     * @brief Parses a DNS query and sends a hijacked A‑record response.
     *
     * Always replies with the AP IP address, regardless of the queried
     * domain name.
     */
    void handleQuery(pbuf *p, const ip_addr_t *addr, u16_t port);

    udp_pcb *pcb = nullptr;
    ip4_addr_t ip;
};
