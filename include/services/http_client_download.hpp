#pragma once
#include <functional>

extern "C" {
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
}

class HttpClientDownload
{
public:
    using ChunkCallback = std::function<void(const uint8_t *, uint32_t)>;
    using DoneCallback = std::function<void(bool)>;

    bool downloadFile(const char *host,
                      uint16_t port,
                      const char *path,
                      const char *filename,
                      ChunkCallback onChunk,
                      DoneCallback onDone);

    void poll();
    void close();

private:
    tcp_pcb *pcb = nullptr;

    struct State
    {
        ChunkCallback onChunk;
        DoneCallback onDone;

        char host[64];
        char path[128];
        char filename[64];
        uint16_t port;

        bool headersDone = false;
    };

    static err_t onConnected(void *arg, tcp_pcb *tpcb, err_t err);
    static err_t onReceive(void *arg, tcp_pcb *tpcb, pbuf *p, err_t err);
    static void onError(void *arg, err_t err);
    static err_t onSent(void *arg, tcp_pcb *tpcb, u16_t len);
};
