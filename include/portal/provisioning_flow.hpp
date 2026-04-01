#pragma once
#include "network/http_server.hpp"

/**
 * @brief High‑level provisioning flow for captive‑portal configuration.
 *
 * This module coordinates the full provisioning sequence:
 *  - starting AP mode
 *  - launching DHCP and DNS hijacking servers
 *  - starting the HTTP server for the configuration portal
 *  - running WiFi scans and injecting results into the HTML
 *  - monitoring for POST /save and triggering a reboot request
 *
 * The flow is split into setup() and loop() to integrate cleanly with
 * the main application without blocking.
 *
 * @note Provisioning is a modal state: once entered, the device remains
 *       in AP mode until the user submits valid WiFi credentials.
 */
static HttpServer http;

/**
 * @brief Initializes all provisioning subsystems.
 *
 * Called once when entering provisioning mode. This function:
 *  - starts the Access Point
 *  - starts DHCP and DNS servers
 *  - starts the HTTP server
 *  - triggers the initial WiFi scan
 */
void provisioning_setup();

/**
 * @brief Periodic update function for the provisioning state machine.
 *
 * Must be called regularly from the main loop. It:
 *  - advances WiFi scanning
 *  - updates the HTTP server (chunked sends, connection handling)
 *  - checks for g_reboot_requested and returns control to the caller
 *
 * @note This function never blocks. All long operations are handled
 *       cooperatively through polling.
 */
void provisioning_loop();
