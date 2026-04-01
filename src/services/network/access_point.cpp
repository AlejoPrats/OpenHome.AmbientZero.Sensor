#include "network/access_point.hpp"
#include "pico/cyw43_arch.h"
#include <cstdio>

extern cyw43_t cyw43_state;

bool AccessPoint::start(const std::string &ssid)
{
    if (cyw43_arch_init())
    {
        return false;
    }

    cyw43_arch_disable_sta_mode();
    sleep_ms(200);

    cyw43_arch_enable_ap_mode(ssid.c_str(), "", CYW43_AUTH_OPEN);

    apNetif = &cyw43_state.netif[CYW43_ITF_AP];

    // Force IP
    ip4addr_aton("192.168.4.1", &apIp);
    ip4_addr_t mask, gw;
    ip4addr_aton("255.255.255.0", &mask);
    ip4addr_aton("192.168.4.1", &gw);
    netif_set_addr(apNetif, &apIp, &mask, &gw);

    return true;
}

void AccessPoint::stop()
{
    cyw43_arch_disable_ap_mode();
}
