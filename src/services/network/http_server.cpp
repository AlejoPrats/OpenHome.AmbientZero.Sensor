#include "network/http_server.hpp"
#include "portal/portal_html.hpp"
#include "pico/cyw43_arch.h"
#include "network/wifi_scan.hpp"
#include "config/device_config.hpp"
#include "hardware/watchdog.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>

std::string g_wifiOptionsHtml;
volatile bool g_reboot_requested = false;

extern "C"
{
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
}

bool HttpServer::start()
{

    buildWifiOptionsHtml();

    pcb = tcp_new();
    if (!pcb)
    {
        return false;
    }

    ip_addr_t ap_ip;
    ip4addr_aton("192.168.4.1", &ap_ip);

    err_t err = tcp_bind(pcb, &ap_ip, 80);
    if (err != ERR_OK)
        return false;

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, HttpServer::onAccept);

    return true;
}

void HttpServer::stop()
{
    if (pcb)
    {
        tcp_close(pcb);
        pcb = nullptr;
    }
}

err_t HttpServer::onAccept(void *arg, tcp_pcb *newPcb, err_t err)
{
    tcp_recv(newPcb, HttpServer::onReceive);
    tcp_err(newPcb, HttpServer::onError);
    tcp_sent(newPcb, HttpServer::onSent);

    return ERR_OK;
}

static std::string applyTemplate(const char *tpl,
                                 const char *body,
                                 const char *wifiOptions = nullptr)
{
    std::string html = tpl;

    // Inject CSS
    if (auto pos = html.find("<!-- PORTAL_STYLE -->"); pos != std::string::npos)
        html.replace(pos, strlen("<!-- PORTAL_STYLE -->"), portal_style);

    // Inject SVG
    if (auto pos = html.find("<!-- LOGO -->"); pos != std::string::npos)
        html.replace(pos, strlen("<!-- LOGO -->"), portal_svg);

    // Inject body (portal or saved)
    if (auto pos = html.find("<!-- PORTAL_BODY -->"); pos != std::string::npos)
        html.replace(pos, strlen("<!-- PORTAL_BODY -->"), body);

    // Inject WiFi options only if provided
    if (wifiOptions)
    {
        if (auto pos = html.find("<!-- WIFI_OPTIONS -->"); pos != std::string::npos)
            html.replace(pos, strlen("<!-- WIFI_OPTIONS -->"), wifiOptions);
    }

    return html;
}

static bool handleCaptivePortalProbes(tcp_pcb *pcb, pbuf *p, const char *buffer)
{
    const bool isAndroid =
        strstr(buffer, "GET /generate_204") ||
        strstr(buffer, "GET /gen_204");

    const bool isApple =
        strstr(buffer, "GET /hotspot-detect.html") ||
        strstr(buffer, "GET /captive.apple.com");

    const bool isWindows =
        strstr(buffer, "GET /connecttest.txt");

    // Windows fallback redirect probe
    if (strstr(buffer, "GET /redirect"))
    {
        const char *resp =
            "HTTP/1.1 302 Found\r\n"
            "Location: http://192.168.4.1/\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";

        pbuf_free(p);
        tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return true;
    }

    const bool isFirefox =
        strstr(buffer, "GET /canonical.html");

    const bool isSteam =
        strncmp(buffer, "GET /204", 8) == 0;

    const bool isMsCrypto =
        strstr(buffer, "GET /msdownload");

    // --- Android & Apple → redirect to portal ---
    if (isAndroid || isApple)
    {
        const char *resp =
            "HTTP/1.1 302 Found\r\n"
            "Location: http://192.168.4.1/\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";

        pbuf_free(p);
        tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return true;
    }

    // --- Windows captive portal ---
    if (isWindows)
    {
        const char *resp =
            "HTTP/1.1 302 Found\r\n"
            "Location: http://192.168.4.1/\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";

        pbuf_free(p);
        tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return true;
    }

    // --- Firefox captive portal ---
    if (isFirefox)
    {
        const char *resp =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 7\r\n"
            "Connection: close\r\n\r\n"
            "success";

        pbuf_free(p);
        tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return true;
    }

    // --- Steam captive portal ---
    if (isSteam)
    {
        const char *resp =
            "HTTP/1.1 204 No Content\r\n"
            "Connection: close\r\n\r\n";

        pbuf_free(p);
        tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return true;
    }

    // --- Windows CryptoAPI certificate fetch ---
    if (isMsCrypto)
    {
        const char *resp =
            "HTTP/1.1 404 Not Found\r\n"
            "Connection: close\r\n\r\n";

        pbuf_free(p);
        tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return true;
    }

    return false; // Not a captive portal probe
}

err_t HttpServer::onReceive(void *arg, tcp_pcb *pcb, pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(pcb);
        return ERR_OK;
    }

    // Tell lwIP we've received this data
    tcp_recved(pcb, p->tot_len);

    char buffer[2048] = {0};
    pbuf_copy_partial(p, buffer, sizeof(buffer) - 1, 0);

    // Captive portal probes → redirect
    if (handleCaptivePortalProbes(pcb, p, buffer))
    {
        return ERR_OK;
    }

    // Favicon → tiny 204
    if (strstr(buffer, "GET /favicon.ico"))
    {
        const char *resp =
            "HTTP/1.1 204 No Content\r\n"
            "Connection: close\r\n\r\n";

        pbuf_free(p);
        err_t e = tcp_write(pcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(pcb);
        return ERR_OK;
    }

    if (strncmp(buffer, "POST /save", 10) == 0)
    {
        return handlePostSave(pcb, buffer, p);
    }

    // Main page
    if (strncmp(buffer, "GET / HTTP", 10) == 0 ||
        strncmp(buffer, "GET / ", 6) == 0 ||
        strstr(buffer, "GET /index.html"))
    {
        pbuf_free(p);

        // 2. Copy template and inject options
        std::string html = applyTemplate(
            portal_index_html,
            portal_body,
            g_wifiOptionsHtml.c_str());

        // 3. Send full HTML in one shot
        startChunkedSend(pcb, html);
        return ERR_OK;
    }

    // Fallback → also serve portal
    pbuf_free(p);
    startChunkedSend(pcb, portal_index_html);
    return ERR_OK;
}

std::string HttpServer::extractField(const char *body, const char *key)
{
    const char *start = strstr(body, key);
    if (!start)
        return "";
    start += strlen(key);

    const char *end = strchr(start, '&');
    if (!end)
        end = start + strlen(start);

    return std::string(start, end - start);
}

void HttpServer::urlDecode(std::string &s)
{
    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == '%' && i + 2 < s.size())
        {
            int val = 0;
            sscanf(s.substr(i + 1, 2).c_str(), "%x", &val);
            out.push_back((char)val);
            i += 2;
        }
        else if (s[i] == '+')
        {
            out.push_back(' ');
        }
        else
        {
            out.push_back(s[i]);
        }
    }

    s = out;
}

err_t HttpServer::handlePostSave(tcp_pcb *pcb, const char *req, pbuf *p)
{
    // Extract body
    const char *body = strstr(req, "\r\n\r\n");
    if (!body)
    {
        pbuf_free(p);
        return ERR_OK;
    }
    body += 4;

    // Example body: ssid=MyWifi&password=12345678
    std::string ssid = extractField(body, "ssid=");
    std::string password = extractField(body, "password=");

    // URL decode
    urlDecode(ssid);
    urlDecode(password);

    // Store in flash
    DeviceConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    load_config(cfg); // If invalid, cfg stays zeroed

    // Update fields
    strncpy(cfg.ssid, ssid.c_str(), sizeof(cfg.ssid) - 1);
    strncpy(cfg.password, password.c_str(), sizeof(cfg.password) - 1);

    // Save to flash
    save_config(cfg);

    // Respond
    std::string html = applyTemplate(
        portal_index_html,
        portal_saved_body);

    pbuf_free(p);
    startChunkedSend(pcb, html);

    // just mark it
    g_reboot_requested = true;

    return ERR_OK;
}

void HttpServer::buildWifiOptionsHtml()
{
    g_wifiOptionsHtml.clear();

    // Sort networks alphabetically by SSID
    std::sort(
        WifiScan::networks,
        WifiScan::networks + WifiScan::networkCount,
        [](const WifiScan::NetworkInfo &a, const WifiScan::NetworkInfo &b)
        {
            return std::string(a.ssid) < std::string(b.ssid);
        });

    for (int i = 0; i < WifiScan::networkCount; ++i)
    {
        const char *ssid = WifiScan::networks[i].ssid;

        std::string safe = ssid;
        size_t pos = 0;
        while ((pos = safe.find('"', pos)) != std::string::npos)
        {
            safe.replace(pos, 1, "&quot;");
            pos += 6;
        }

        g_wifiOptionsHtml += "<option value=\"";
        g_wifiOptionsHtml += safe;
        g_wifiOptionsHtml += "\">";
        g_wifiOptionsHtml += safe;
        g_wifiOptionsHtml += "</option>\n";
    }
}

err_t HttpServer::sendResponse(tcp_pcb *pcb, const char *body)
{
    static SendState *state = new SendState{body, strlen(body)};

    char header[256];
    int body_len = strlen(body);

    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html\r\n"
                              "Content-Length: %d\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              body_len);

    err_t err = tcp_write(pcb, header, header_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);

    // Try to send first chunk
    HttpServer::continueSend(pcb, state);

    // Attach state to PCB
    tcp_arg(pcb, state);

    return ERR_OK;
}

void HttpServer::continueSend(tcp_pcb *pcb, void *arg)
{
    SendState *state = static_cast<SendState *>(arg);
    while (state->offset < state->data.size())
    {
        size_t remaining = state->data.size() - state->offset;
        u16_t chunk = remaining > 512 ? 512 : (u16_t)remaining;

        const char *ptr = state->data.c_str() + state->offset;
        err_t err = tcp_write(pcb, ptr, chunk, TCP_WRITE_FLAG_COPY);

        if (err == ERR_MEM)
        {
            tcp_output(pcb);
            return; // wait for onSent()
        }

        if (err != ERR_OK)
        {
            tcp_abort(pcb);
            return;
        }

        state->offset += chunk;
    }

    // All data sent → close connection
    tcp_output(pcb);
    tcp_close(pcb);
}

void HttpServer::sendRaw(tcp_pcb *pcb, const char *data, size_t len, const char *contentType)
{
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %u\r\n"
             "Connection: close\r\n\r\n",
             contentType, (unsigned)len);

    err_t e = tcp_write(pcb, header, strlen(header), TCP_WRITE_FLAG_COPY);

    if (e == ERR_OK)
    {
        e = tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY);
    }

    tcp_output(pcb);
}

void HttpServer::onError(void *arg, err_t err)
{
    printf("HttpServer: connection error %d\n", err);
}

err_t HttpServer::onSent(void *arg, tcp_pcb *pcb, u16_t len)
{
    SendState *state = (SendState *)arg;
    if (!state)
        return ERR_OK;

    continueSend(pcb, state);

    // If all chunks are sent, request reboot
    if (state->offset >= state->data.size())
    {
        // (later delete state here)
    }

    return ERR_OK;
}

void HttpServer::startChunkedSend(tcp_pcb *pcb, const std::string &body)
{
    // 1. Build header
    char header[256];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              body.size());

    // 2. Allocate send state
    SendState *state = new SendState{
        .data = body,
        .offset = 0};

    // 3. Attach state to PCB
    tcp_arg(pcb, state);
    tcp_sent(pcb, HttpServer::onSent);

    // 4. Send header first (always fits)
    tcp_write(pcb, header, header_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);

    // 5. Kick off first chunk
    continueSend(pcb, state);
}
