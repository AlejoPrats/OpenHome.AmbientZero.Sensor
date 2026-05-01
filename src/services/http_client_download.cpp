#include "services/http_client_download.hpp"
#include <lwip/tcp.h>
#include <cstring>

bool HttpClientDownload::downloadFile(const char* host,
                                      uint16_t port,
                                      const char* path,
                                      const char* filename,
                                      ChunkCallback onChunk,
                                      DoneCallback onDone)
{
    tcp_pcb* pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    this->pcb = pcb;
    if (!pcb)
        return false;

    auto* state = new State{};
    state->onChunk = onChunk;
    state->onDone  = onDone;
    state->port    = port;

    std::strncpy(state->host, host, sizeof(state->host));
    state->host[sizeof(state->host) - 1] = '\0';

    std::strncpy(state->path, path, sizeof(state->path));
    state->path[sizeof(state->path) - 1] = '\0';

    std::strncpy(state->filename, filename, sizeof(state->filename));
    state->filename[sizeof(state->filename) - 1] = '\0';

    tcp_arg(pcb, state);
    tcp_recv(pcb, onReceive);
    tcp_err(pcb, onError);
    tcp_sent(pcb, onSent);

    ip_addr_t addr;
    if (!ip4addr_aton(host, ip_2_ip4(&addr)))
    {
        tcp_abort(pcb);
        delete state;
        this->pcb = nullptr;
        return false;
    }

    err_t err = tcp_connect(pcb, &addr, port, onConnected);
    if (err != ERR_OK)
    {
        tcp_abort(pcb);
        delete state;
        this->pcb = nullptr;
    }
    return err == ERR_OK;
}

void HttpClientDownload::poll() {}

void HttpClientDownload::close()
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
}

err_t HttpClientDownload::onConnected(void* arg, tcp_pcb* tpcb, err_t err)
{
    auto* state = static_cast<State*>(arg);

    if (err != ERR_OK)
    {
        if (state && state->onDone)
            state->onDone(false);

        tcp_close(tpcb);
        delete state;
        return err;
    }

    char req[256];
    int n = snprintf(req, sizeof(req),
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s:%u\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     state->path, state->host, state->port);

    tcp_write(tpcb, req, n, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    return ERR_OK;
}

err_t HttpClientDownload::onReceive(void* arg, tcp_pcb* tpcb, pbuf* p, err_t err)
{
    auto* state = static_cast<State*>(arg);

    if (!p)
    {
        if (state && state->onDone)
            state->onDone(true);

        tcp_close(tpcb);
        delete state;
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(p);

        if (state && state->onDone)
            state->onDone(false);

        tcp_close(tpcb);
        delete state;
        return err;
    }

    char* buf = new char[p->tot_len + 1];
    pbuf_copy_partial(p, buf, p->tot_len, 0);
    buf[p->tot_len] = '\0';

    char* data = buf;
    uint32_t len = p->tot_len;

    if (!state->headersDone)
    {
        char* body = std::strstr(buf, "\r\n\r\n");
        if (body)
        {
            body += 4;
            uint32_t headerLen = body - buf;
            uint32_t bodyLen   = len - headerLen;
            state->headersDone = true;

            if (state->onChunk && bodyLen > 0)
                state->onChunk(reinterpret_cast<uint8_t*>(body), bodyLen);
        }
    }
    else
    {
        if (state->onChunk && len > 0)
            state->onChunk(reinterpret_cast<uint8_t*>(data), len);
    }

    delete[] buf;
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

void HttpClientDownload::onError(void* arg, err_t err)
{
    auto* state = static_cast<State*>(arg);

    if (state && state->onDone)
        state->onDone(false);

    delete state;
}

err_t HttpClientDownload::onSent(void* arg, tcp_pcb* tpcb, u16_t len)
{
    return ERR_OK;
}
