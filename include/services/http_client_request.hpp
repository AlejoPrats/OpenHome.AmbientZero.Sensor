#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include "services/http_request.hpp"
#include <lwip/tcp.h>
#include <lwip/pbuf.h>

class HttpClientRequest
{
public:
    using ChunkCallback = std::function<void(const uint8_t*, uint32_t)>;
    using DoneCallback  = std::function<void(bool)>;

    bool request(const HttpJsonRequest& req,
                 ChunkCallback onChunk,
                 DoneCallback  onDone);

    void poll();
    void close();

private:
    tcp_pcb*        pcb = nullptr;
    HttpJsonRequest currentReq{};
    ChunkCallback   onChunk_{};
    DoneCallback    onDone_{};
    uint32_t        startTimeMs = 0;

    static err_t onConnected(void* arg, tcp_pcb* tpcb, err_t err);
    static err_t onReceive(void* arg, tcp_pcb* tpcb, pbuf* p, err_t err);
    static void  onError(void* arg, err_t err);
    static err_t onSent(void* arg, tcp_pcb* tpcb, u16_t len);
};
