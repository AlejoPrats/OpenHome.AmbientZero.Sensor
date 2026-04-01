#pragma once

#include <stdint.h>
#include <string>
#include <functional>

/**
 * @brief Sends an HTTP request using raw lwIP sockets.
 *
 * This function performs a non‑blocking HTTP transaction to the
 * specified IP and port. It constructs the request manually
 * (verb, path, headers, and optional body) and delivers the
 * server's full response body to the provided callback.
 *
 * @param ip        Target IPv4 address as a null‑terminated string.
 * @param port      Destination TCP port.
 * @param verb      HTTP method (e.g., "GET", "POST", "PUT", "DELETE").
 * @param path      Request path (e.g., "/api/data").
 * @param body      Optional request body. May be empty.
 * @param callback  Function invoked when the full response is received.
 *
 * @note This function does not block. All network progress occurs
 *       through lwIP polling and must be driven by http_poll().
 *
 * @warning The callback is executed from the network context.
 *          It must be lightweight and non‑blocking.
 */
void http_request(const char *ip,
                  uint16_t port,
                  const char *verb,
                  const char *path,
                  const std::string &body,
                  std::function<void(const std::string &)> callback);

/**
 * @brief Convenience overload for HTTP methods without a body.
 *
 * This overload is intended for GET, DELETE, and other verbs that
 * do not require a request payload.
 */
inline void http_request(const char *ip,
                         uint16_t port,
                         const char *verb,
                         const char *path,
                         std::function<void(const std::string &)> callback)
{
    http_request(ip, port, verb, path, "", callback);
}

/**
 * @brief Advances all pending HTTP transactions.
 *
 * This function must be called periodically from the main loop.
 * It drives lwIP timers, TCP state machines, and response parsing.
 *
 * @note If http_poll() is not called frequently enough, requests
 *       may stall or time out.
 */
void http_poll();
