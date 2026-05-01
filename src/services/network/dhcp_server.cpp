#include "network/dhcp_server.hpp"
#include <cstdio>

extern "C" {
    #include "network/dhcp_server_c.h"
}

bool DhcpServer::start(struct netif* apNetif) {
    dhcpd_start(apNetif);
    return true;
}

void DhcpServer::stop() {
    dhcpd_stop();
}

void DhcpServer::recvCallback(void* arg, struct udp_pcb* pcb,
                              struct pbuf* p, const ip_addr_t* addr, u16_t port) {
    // TODO: forward to C DHCP handler
}
