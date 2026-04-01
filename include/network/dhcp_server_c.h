#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"

/**
 * @brief Start the lightweight DHCP server.
 *
 * This function binds UDP port 67 on the given netif and begins responding
 * to DHCPDISCOVER and DHCPREQUEST packets. The server assigns a single
 * static lease (server_ip + 1).
 *
 * @param netif Pointer to the AP network interface (must be valid and UP).
 */
void dhcpd_start(struct netif *netif);

/**
 * @brief Stop the DHCP server and release its UDP PCB.
 */
void dhcpd_stop(void);

#ifdef __cplusplus
}
#endif
