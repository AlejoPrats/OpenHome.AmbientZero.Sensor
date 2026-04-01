#include "network/dhcp_server_c.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/ip4_addr.h"
#include "lwip/inet.h"
#include <string.h>
#include <stdio.h>

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

#define DHCP_OP_BOOTREQUEST 1
#define DHCP_OP_BOOTREPLY 2

#define DHCP_HTYPE_ETH 1
#define DHCP_HLEN_ETH 6

#define DHCP_MAGIC_COOKIE 0x63825363

#define DHCP_OPTION_MSG_TYPE 53
#define DHCP_OPTION_REQ_IP 50
#define DHCP_OPTION_SERVER_ID 54
#define DHCP_OPTION_END 255

#define DHCPDISCOVER 1
#define DHCPOFFER 2
#define DHCPREQUEST 3
#define DHCPACK 5

#pragma pack(push, 1)
struct dhcp_msg
{
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint32_t cookie;
    uint8_t options[312];
};
#pragma pack(pop)

static struct udp_pcb *dhcp_pcb = NULL;
static struct netif *dhcp_netif = NULL;
static ip4_addr_t server_ip;
static ip4_addr_t lease_ip; // fixed: server_ip + 1

static uint8_t get_msg_type(const struct dhcp_msg *msg, int len)
{
    const uint8_t *opt = msg->options;
    int remain = len - (int)((const uint8_t *)msg->options - (const uint8_t *)msg);
    while (remain > 0)
    {
        uint8_t code = *opt++;
        if (code == DHCP_OPTION_END)
            break;
        if (code == 0)
        {
            remain--;
            continue;
        }
        if (remain < 2)
            break;
        uint8_t optlen = *opt++;
        remain -= 2;
        if (optlen > remain)
            break;
        if (code == DHCP_OPTION_MSG_TYPE && optlen == 1)
        {
            return *opt;
        }
        opt += optlen;
        remain -= optlen;
    }
    return 0;
}

static void add_option(uint8_t **opt, uint8_t code, uint8_t len, const void *data)
{
    *(*opt)++ = code;
    *(*opt)++ = len;
    memcpy(*opt, data, len);
    *opt += len;
}

static void add_option_u8(uint8_t **opt, uint8_t code, uint8_t val)
{
    *(*opt)++ = code;
    *(*opt)++ = 1;
    *(*opt)++ = val;
}

static void build_reply(struct dhcp_msg *reply,
                        const struct dhcp_msg *req,
                        uint8_t msg_type)
{
    memset(reply, 0, sizeof(*reply));
    reply->op = DHCP_OP_BOOTREPLY;
    reply->htype = DHCP_HTYPE_ETH;
    reply->hlen = DHCP_HLEN_ETH;
    reply->xid = req->xid;
    reply->flags = req->flags;
    memcpy(reply->chaddr, req->chaddr, 16);
    reply->cookie = htonl(DHCP_MAGIC_COOKIE);

    reply->yiaddr = lease_ip.addr;
    reply->siaddr = server_ip.addr;

    uint8_t *opt = reply->options;
    add_option_u8(&opt, DHCP_OPTION_MSG_TYPE, msg_type);
    add_option(&opt, 54, 4, &server_ip.addr); // server identifier
    // lease time: 1 hour
    uint32_t lease = htonl(3600);
    add_option(&opt, 51, 4, &lease);
    // subnet mask: 255.255.255.0
    ip4_addr_t netmask;
    ip4addr_aton("255.255.255.0", &netmask);
    add_option(&opt, 1, 4, &netmask.addr);
    // router: server_ip
    add_option(&opt, 3, 4, &server_ip.addr);
    // DNS: server_ip (for captive portal)
    add_option(&opt, 6, 4, &server_ip.addr);
    *opt++ = DHCP_OPTION_END;
}

static void dhcpd_recv(void *arg, struct udp_pcb *pcb,
                       struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (!p)
        return;

    size_t min_len = sizeof(struct dhcp_msg) - sizeof(((struct dhcp_msg *)0)->options);

    if (p->len < min_len)
    {
        pbuf_free(p);
        return;
    }

    struct dhcp_msg req;
    pbuf_copy_partial(p, &req, sizeof(req), 0);
    if (req.op != DHCP_OP_BOOTREQUEST ||
        req.htype != DHCP_HTYPE_ETH ||
        req.hlen != DHCP_HLEN_ETH ||
        ntohl(req.cookie) != DHCP_MAGIC_COOKIE)
    {
        pbuf_free(p);
        return;
    }

    uint8_t mtype = get_msg_type(&req, p->len);

    if (mtype != DHCPDISCOVER && mtype != DHCPREQUEST)
    {
        pbuf_free(p);
        return;
    }

    struct dhcp_msg rep;
    uint8_t rep_type = (mtype == DHCPDISCOVER) ? DHCPOFFER : DHCPACK;
    build_reply(&rep, &req, rep_type);

    struct pbuf *q = pbuf_alloc(PBUF_TRANSPORT, sizeof(rep), PBUF_POOL);
    if (!q)
    {
        pbuf_free(p);
        return;
    }
    pbuf_take(q, &rep, sizeof(rep));

    ip_addr_t dest;
    ip_addr_set_ip4_u32(&dest, IPADDR_BROADCAST);

    udp_sendto_if(pcb, q, &dest, DHCP_CLIENT_PORT, dhcp_netif);

    pbuf_free(q);
    pbuf_free(p);
}

void dhcpd_start(struct netif *netif)
{
    if (dhcp_pcb)
        return;

    dhcp_netif = netif;

    // CYW43 AP mode ALWAYS uses 192.168.4.1
    ip4addr_aton("192.168.4.1", &server_ip);

    // Lease = 192.168.4.2
    ip4_addr_set_u32(&lease_ip, htonl(ntohl(server_ip.addr) + 1));

    dhcp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!dhcp_pcb)
    {
        return;
    }

    if (udp_bind(dhcp_pcb, IP_ANY_TYPE, DHCP_SERVER_PORT) != ERR_OK)
    {
        udp_remove(dhcp_pcb);
        dhcp_pcb = NULL;
        return;
    }

    udp_recv(dhcp_pcb, dhcpd_recv, NULL);

    char buf_server[16];
    char buf_lease[16];

    ip4addr_ntoa_r(&server_ip, buf_server, sizeof(buf_server));
    ip4addr_ntoa_r(&lease_ip, buf_lease, sizeof(buf_lease));
}

void dhcpd_stop(void)
{
    if (dhcp_pcb)
    {
        udp_remove(dhcp_pcb);
        dhcp_pcb = NULL;
    }
}
