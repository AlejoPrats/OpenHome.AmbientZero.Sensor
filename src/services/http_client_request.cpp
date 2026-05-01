#include "services/http_client_request.hpp"
#include "pico/stdlib.h"
#include "lwip/ip_addr.h"

bool HttpClientRequest::request(const HttpJsonRequest& req,
                                ChunkCallback onChunk,
                                DoneCallback  onDone)
{
    if (pcb != nullptr)
        return false; // already busy

    currentReq = req;
    onChunk_   = onChunk;
    onDone_    = onDone;

    ip_addr_t addr;
    if (!ipaddr_aton(req.ip, &addr))
        return false;

    pcb = tcp_new();
    if (!pcb)
        return false;

    startTimeMs = to_ms_since_boot(get_absolute_time());

    tcp_arg(pcb, this);
    tcp_recv(pcb, &HttpClientRequest::onReceive);
    tcp_err(pcb, &HttpClientRequest::onError);
    tcp_sent(pcb, &HttpClientRequest::onSent);

    err_t err = tcp_connect(pcb, &addr, req.port, &HttpClientRequest::onConnected);
    if (err != ERR_OK)
    {
        tcp_abort(pcb);
        pcb = nullptr;
        return false;
    }

    return true;
}

void HttpClientRequest::poll()
{
    if (!pcb)
        return;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t timeout = currentReq.timeoutMs;

    if (timeout > 0 && (now - startTimeMs > timeout))
    {
        tcp_abort(pcb);
        pcb = nullptr;

        if (onDone_)
            onDone_(false);

        currentReq = HttpJsonRequest{};
        onChunk_   = nullptr;
        onDone_    = nullptr;
        startTimeMs = 0;
    }
}

void HttpClientRequest::close()
{
    if (pcb)
    {
        tcp_arg(pcb, nullptr);
        tcp_recv(pcb, nullptr);
        tcp_err(pcb, nullptr);
        tcp_sent(pcb, nullptr);
        tcp_abort(pcb);
        pcb = nullptr;
    }

    currentReq = HttpJsonRequest{};
    onChunk_   = nullptr;
    onDone_    = nullptr;
    startTimeMs = 0;
}

// -----------------------------
// Static callbacks
// -----------------------------

err_t HttpClientRequest::onConnected(void* arg, tcp_pcb* tpcb, err_t err)
{
    auto* self = static_cast<HttpClientRequest*>(arg);
    if (!self || err != ERR_OK)
        return err;

    const auto& req = self->currentReq;

    std::string request;
    request.reserve(256 + req.body.size());

    request += req.verb;
    request += " ";
    request += req.path;
    request += " HTTP/1.1\r\n";
    request += "Host: ";
    request += req.ip;
    request += "\r\n";
    request += "Connection: close\r\n";

    if (!req.body.empty())
    {
        request += "Content-Length: ";
        request += std::to_string(req.body.size());
        request += "\r\nContent-Type: application/json\r\n";
    }

    request += "\r\n";

    if (!req.body.empty())
        request += req.body;

    tcp_write(tpcb, request.c_str(), request.size(), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    return ERR_OK;
}

err_t HttpClientRequest::onReceive(void* arg, tcp_pcb* tpcb, pbuf* p, err_t err)
{
    auto* self = static_cast<HttpClientRequest*>(arg);
    if (!self)
        return ERR_OK;

    if (!p)
    {
        if (self->onDone_)
            self->onDone_(true);

        self->close();
        return ERR_OK;
    }

    if (self->onChunk_)
        self->onChunk_(static_cast<const uint8_t*>(p->payload), p->len);

    tcp_recved(tpcb, p->len);
    pbuf_free(p);
    return ERR_OK;
}

void HttpClientRequest::onError(void* arg, err_t err)
{
    auto* self = static_cast<HttpClientRequest*>(arg);
    if (!self)
        return;

    if (self->onDone_)
        self->onDone_(false);

    self->pcb = nullptr;
    self->currentReq = HttpJsonRequest{};
    self->onChunk_   = nullptr;
    self->onDone_    = nullptr;
    self->startTimeMs = 0;
}

err_t HttpClientRequest::onSent(void* arg, tcp_pcb* tpcb, u16_t len)
{
    return ERR_OK;
}
