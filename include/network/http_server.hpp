#pragma once

#include <lwip/tcp.h>
#include <string>

extern std::string g_wifiOptionsHtml;
extern volatile bool g_reboot_requested;

/**
 * @brief Minimal HTTP server used for the provisioning captive portal.
 *
 * This server handles a very small subset of HTTP sufficient for:
 *  - serving the provisioning HTML page
 *  - responding to captive‑portal probes
 *  - processing POST requests containing WiFi credentials
 *  - triggering a device reboot after successful configuration
 *
 * It uses lwIP’s raw TCP API and implements a simple request parser
 * with chunked sending support for large responses.
 *
 * @note This server is intentionally minimal and not suitable for
 *       general‑purpose HTTP workloads.
 */
class HttpServer
{
public:
    /**
     * @brief Starts the HTTP server on TCP port 80.
     *
     * Allocates a listening PCB, binds it to port 80, and registers
     * the accept callback. After start(), the server will handle all
     * incoming connections until stop() is called.
     *
     * @return true if the server was successfully started.
     */
    bool start();

    /**
     * @brief Stops the HTTP server and releases its listening PCB.
     *
     * Active connections may still complete their send operations,
     * but no new connections will be accepted.
     */
    void stop();

private:
    // ---------------------------------------------------------------------
    // lwIP callbacks
    // ---------------------------------------------------------------------

    /**
     * @brief Called by lwIP when a new TCP connection is accepted.
     *
     * Creates per‑connection state and registers receive/sent/error
     * callbacks for the new PCB.
     */
    static err_t onAccept(void *arg, tcp_pcb *newPcb, err_t err);

    /**
     * @brief Called when data is received on a connection.
     *
     * Parses the HTTP request, dispatches GET/POST handlers, and
     * initiates the appropriate response.
     */
    static err_t onReceive(void *arg, tcp_pcb *pcb, pbuf *p, err_t err);

    /**
     * @brief Called when a connection encounters a fatal error.
     *
     * Cleans up any per‑connection state.
     */
    static void onError(void *arg, err_t err);

    /**
     * @brief Called after data has been successfully sent.
     *
     * Used to continue chunked responses until all data is transmitted.
     */
    static err_t onSent(void *arg, tcp_pcb *pcb, u16_t len);

    // ---------------------------------------------------------------------
    // Response helpers
    // ---------------------------------------------------------------------

    /**
     * @brief Sends a complete HTTP response with a simple body.
     *
     * For larger responses, startChunkedSend() is used instead.
     */
    static err_t sendResponse(tcp_pcb *pcb, const char *body);

    /**
     * @brief Continues sending a chunked response.
     *
     * Called from onSent() until the entire body has been transmitted.
     */
    static void continueSend(tcp_pcb *pcb, void *arg);

    /**
     * @brief Builds the HTML snippet containing scanned WiFi networks.
     *
     * Populates g_wifiOptionsHtml with <option> entries.
     */
    static void buildWifiOptionsHtml();

    /**
     * @brief Extracts a form field from a POST body.
     *
     * Searches for key=value pairs and returns the decoded value.
     */
    static std::string extractField(const char *body, const char *key);

    /**
     * @brief Decodes URL‑encoded characters in‑place.
     */
    static void urlDecode(std::string &s);

    /**
     * @brief Handles POST /save requests containing WiFi credentials.
     *
     * Saves the configuration, sends a confirmation page, and sets
     * g_reboot_requested so the main loop can reboot safely.
     */
    static err_t handlePostSave(tcp_pcb *pcb, const char *req, pbuf *p);

    /**
     * @brief Sends raw data with a specified Content‑Type.
     */
    static void sendRaw(tcp_pcb *pcb, const char *data, size_t len, const char *contentType);

    /**
     * @brief Begins a chunked HTTP response for large bodies.
     *
     * Allocates per‑connection SendState and sends the first chunk.
     */
    static void startChunkedSend(tcp_pcb *pcb, const std::string &body);

    // ---------------------------------------------------------------------
    // Per‑connection send state
    // ---------------------------------------------------------------------

    /**
     * @brief Tracks progress of a chunked send operation.
     */
    struct SendState
    {
        std::string data;  ///< Full response body
        size_t offset;     ///< Bytes already sent
    };

    tcp_pcb *pcb = nullptr;  ///< Listening PCB
};
