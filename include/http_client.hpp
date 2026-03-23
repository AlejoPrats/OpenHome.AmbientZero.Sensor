#pragma once

#include <stdint.h>
#include <string>
#include <functional>

// Send an HTTP request using RAW lwIP.
// Example:
//   http_request("192.168.1.50", 8080, "POST", "/api", "hello", callback);
void http_request(const char* ip,
                  uint16_t port,
                  const char* verb,
                  const char* path,
                  const std::string& body,
                  std::function<void(const std::string&)> callback);

// Overload for GET/DELETE with no body
inline void http_request(const char* ip,
                         uint16_t port,
                         const char* verb,
                         const char* path,
                         std::function<void(const std::string&)> callback) {
    http_request(ip, port, verb, path, "", callback);
}

// Must be called periodically in your main loop
void http_poll();
