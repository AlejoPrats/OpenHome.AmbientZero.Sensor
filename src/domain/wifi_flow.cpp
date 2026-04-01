#include "domain/wifi_flow.hpp"
#include "services/wifi.hpp"

#include "network/access_point.hpp"
#include "network/dhcp_server.hpp"
#include "network/dns_server.hpp"
#include "network/wifi_scan.hpp"

#include "pico/stdlib.h"

#include <cstdio>

namespace WifiFlow
{
    // Private to this namespace — not visible outside
    static AccessPoint g_ap;
    static DhcpServer g_dhcp;
    static DnsServer g_dns;

    bool wifi_connect_flow(const DeviceConfig &cfg)
    {
        const char* ssid = (cfg.ssid[0] == '\0') ? "AmbientZero" : cfg.ssid;
        const char* password = (cfg.ssid[0] == '\0') ? "" : cfg.password;

        WiFi wifi(ssid, password);

        if (!wifi.connect())
        {
            return false;
        }

        return true;
    }

    bool startAccessPoint(const std::string &ssid)
    {
        if (!g_ap.start(ssid))
        {
            return false;
        }

        struct netif *apNetif = g_ap.getNetif();
        ip4_addr_t apIp = g_ap.getIp();

        if (!apNetif)
        {
            return false;
        }

        if (!g_dhcp.start(apNetif))
        {
            return false;
        }

        if (!g_dns.start(apIp))
        {
            return false;
        }

        return true;
    }

    void scanNetworks(int passes)
    {
        WifiScan::start(passes);
        while (!WifiScan::isFinished())
        {
            WifiScan::poll();
            sleep_ms(50);
        }
    }

} // namespace WifiFlow