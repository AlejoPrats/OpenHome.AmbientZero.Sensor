#pragma once
#include <string>
#include <functional>

struct HttpRequestBase {
    const char* ip;
    uint16_t    port;
    const char* path;
};

struct HttpJsonRequest : public HttpRequestBase {
    const char* verb;
    std::string body;
    std::function<void(const std::string&)> callback;

    uint32_t timeoutMs = 30000;   // default 30 seconds
};

struct HttpDownloadRequest : public HttpRequestBase {
    const char* filename;
    std::function<void(const uint8_t*, uint32_t)> onChunk;
    std::function<void(bool)> onDone;

    uint32_t timeoutMs = 120000;  // default 120 seconds
};
