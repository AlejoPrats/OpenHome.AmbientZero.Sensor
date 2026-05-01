#include "services/http_client.hpp"
#include "pico/stdlib.h"

std::string HttpClient::request(const HttpJsonRequest& req)
{
    close(); // ensure clean state

    mode = Mode::JsonSync;
    syncDone = false;
    syncSuccess = false;
    syncResponse.clear();

    bool ok = requestImpl.request(
        req,
        // onChunk
        [this](const uint8_t* data, uint32_t len)
        {
            syncResponse.append(reinterpret_cast<const char*>(data), len);
        },
        // onDone
        [this](bool success)
        {
            syncSuccess = success;
            syncDone = true;
        });

    if (!ok)
    {
        close();
        return "";
    }

    uint32_t start = to_ms_since_boot(get_absolute_time());
    uint32_t timeout = req.timeoutMs;

    while (!syncDone)
    {
        pollInternal();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - start > timeout)
        {
            close();
            return "";
        }
        sleep_ms(2); // battery-friendly
    }

    close();
    return syncSuccess ? syncResponse : "";
}

void HttpClient::poll()
{
    pollInternal();
}


bool HttpClient::request_async(const HttpJsonRequest& req,
                              ChunkCallback onChunk,
                              DoneCallback onDone)
{
    close();

    mode = Mode::JsonAsync;

    return requestImpl.request(req, onChunk, onDone);
}

bool HttpClient::download_file(const HttpDownloadRequest& req)
{
    close();

    mode = Mode::Download;

    return downloadImpl.downloadFile(
        req.ip,
        req.port,
        req.path,
        req.filename,
        req.onChunk,
        req.onDone);
}


void HttpClient::pollInternal()
{
    switch (mode)
    {
        case Mode::JsonAsync:
        case Mode::JsonSync:
            requestImpl.poll();
            break;

        case Mode::Download:
            downloadImpl.poll();
            break;

        default:
            break;
    }
}

void HttpClient::close()
{
    switch (mode)
    {
        case Mode::JsonAsync:
        case Mode::JsonSync:
            requestImpl.close();
            break;

        case Mode::Download:
            downloadImpl.close();
            break;

        default:
            break;
    }

    mode = Mode::None;
}
