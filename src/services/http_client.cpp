#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string>
#include <functional>

static struct tcp_pcb* http_pcb = nullptr;
static std::string http_response;
static std::function<void(const std::string&)> http_callback;

static uint16_t http_port = 80;
static std::string http_ip;
static std::string http_verb;
static std::string http_path;
static std::string http_body;

static uint32_t http_start_time = 0;
static uint32_t http_timeout_ms = 30000; // default 30 seconds

// -----------------------------
// TCP Callbacks
// -----------------------------

static err_t http_recv(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    http_response.append((char*)p->payload, p->len);

    tcp_recved(tpcb, p->len);
    pbuf_free(p);
    return ERR_OK;
}

static err_t http_connected(void* arg, struct tcp_pcb* tpcb, err_t err) {
    if (err != ERR_OK) {
        tcp_close(tpcb);
        return err;
    }

    // Build HTTP request
    std::string request;
    request.reserve(256 + http_body.size());

    request += http_verb + " " + http_path + " HTTP/1.1\r\n";
    request += "Host: " + http_ip + "\r\n";
    request += "Connection: close\r\n";

    if (!http_body.empty()) {
        request += "Content-Length: " + std::to_string(http_body.size()) + "\r\n";
        request += "Content-Type: application/json\r\n";
    }

    request += "\r\n";

    if (!http_body.empty()) {
        request += http_body;
    }

    tcp_write(tpcb, request.c_str(), request.size(), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    return ERR_OK;
}

// -----------------------------
// Public API
// -----------------------------

void http_request(const char* ip,
                  uint16_t port,
                  const char* verb,
                  const char* path,
                  const std::string& body,
                  std::function<void(const std::string&)> callback) {
    http_callback = callback;
    http_response.clear();

    http_ip   = ip;
    http_port = port;
    http_verb = verb;
    http_path = path;
    http_body = body;

    ip_addr_t addr;
    if (!ipaddr_aton(ip, &addr)) {
        return;
    }

    http_pcb = tcp_new();
    if (!http_pcb) {
        return;
    }

    tcp_recv(http_pcb, http_recv);

    tcp_connect(http_pcb, &addr, http_port, http_connected);
}

void http_poll() {
    if (!http_pcb) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // TIMEOUT CHECK
    if (now - http_start_time > http_timeout_ms) {
        tcp_abort(http_pcb);   // force close
        http_pcb = nullptr;

        if (http_callback) {
            http_callback(""); // empty response = timeout
        }
        return;
    }

    // NORMAL CLOSE CHECK
    if (http_pcb->state == CLOSED || http_pcb->state == TIME_WAIT) {
        if (http_callback) {
            http_callback(http_response);
        }
        http_pcb = nullptr;
    }
}
