#include "network/dns_server.hpp"
#include <cstdio>
#include <cstring>

extern "C" {
    #include "lwip/udp.h"
    #include "lwip/pbuf.h"
}

bool DnsServer::start(ip4_addr_t apIp) {
    ip = apIp;

    pcb = udp_new();
    if (!pcb) {
        return false;
    }

    err_t err = udp_bind(pcb, IP_ANY_TYPE, 53);
    if (err != ERR_OK) {
        return false;
    }

    udp_recv(pcb, DnsServer::recvCallback, this);

    return true;
}

void DnsServer::stop() {
    if (pcb) {
        udp_remove(pcb);
        pcb = nullptr;
    }
}

void DnsServer::recvCallback(void* arg, udp_pcb* pcb,
                             pbuf* p, const ip_addr_t* addr, u16_t port)
{
    DnsServer* self = static_cast<DnsServer*>(arg);
    self->handleQuery(p, addr, port);
}

void DnsServer::handleQuery(pbuf* p, const ip_addr_t* addr, u16_t port)
{
    if (!p) return;

    uint8_t buffer[512];
    pbuf_copy_partial(p, buffer, sizeof(buffer), 0);

    // Build DNS response
    uint8_t response[512];
    memset(response, 0, sizeof(response));

    // Transaction ID
    response[0] = buffer[0];
    response[1] = buffer[1];

    // Flags: standard query response, no error
    response[2] = 0x81;
    response[3] = 0x80;

    // Questions = 1
    response[4] = 0x00;
    response[5] = 0x01;

    // Answers = 1
    response[6] = 0x00;
    response[7] = 0x01;

    // Copy original query
    int query_len = p->tot_len - 12;
    memcpy(&response[12], &buffer[12], query_len);

    int pos = 12 + query_len;

    // Answer: pointer to name
    response[pos++] = 0xC0;
    response[pos++] = 0x0C;

    // Type A
    response[pos++] = 0x00;
    response[pos++] = 0x01;

    // Class IN
    response[pos++] = 0x00;
    response[pos++] = 0x01;

    // TTL
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x3C;

    // Data length = 4
    response[pos++] = 0x00;
    response[pos++] = 0x04;

    // IP address
    memcpy(&response[pos], &ip.addr, 4);
    pos += 4;

    // Send response
    pbuf* resp = pbuf_alloc(PBUF_TRANSPORT, pos, PBUF_RAM);
    memcpy(resp->payload, response, pos);

    udp_sendto(pcb, resp, addr, port);
    pbuf_free(resp);
    pbuf_free(p);
}
