#pragma once

/**
 * @brief Embedded HTML, CSS, and SVG assets for the provisioning portal.
 *
 * These pointers reference static, null‑terminated strings containing
 * the HTML/CSS/SVG content served by the captive‑portal HTTP server.
 * The actual definitions live in a corresponding .cpp file where the
 * assets are stored in flash.
 *
 * Assets:
 *  - portal_style:       <style> block used by all portal pages
 *  - portal_body:        Main provisioning form (SSID list, password field)
 *  - portal_svg:         Inline SVG logo used by the UI
 *  - portal_saved_body:  Confirmation page shown after saving WiFi config
 *  - portal_index_html:  Full index.html served for GET /
 *
 * @note These are raw strings, not templates. Any dynamic content
 *       (e.g., WiFi network list) is injected by the HTTP server.
 */
extern const char* portal_style;
extern const char* portal_body;
extern const char* portal_svg;
extern const char* portal_saved_body;
extern const char* portal_index_html;
