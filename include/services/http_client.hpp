#pragma once
#include <string>
#include <functional>
#include "services/http_client_request.hpp"
#include "services/http_client_download.hpp"
#include "services/http_request.hpp"

class HttpClient
{
public:
    using ChunkCallback = std::function<void(const uint8_t*, uint32_t)>;
    using DoneCallback  = std::function<void(bool)>;

    // Synchronous JSON request (blocking)
    std::string request(const HttpJsonRequest& req);

    // Asynchronous JSON request
    bool request_async(const HttpJsonRequest& req,
                      ChunkCallback onChunk,
                      DoneCallback onDone);

    // OTA / file download (async only)
    bool download_file(const HttpDownloadRequest& req);

    void poll();

    // Abort/cleanup
    void close();

private:
    enum class Mode {
        None,
        JsonAsync,
        JsonSync,
        Download
    };

    Mode mode = Mode::None;

    HttpClientRequest  requestImpl;
    HttpClientDownload downloadImpl;

    // Sync JSON state
    bool syncDone = false;
    bool syncSuccess = false;
    std::string syncResponse;

    // Internal helpers
    void pollInternal();
};
