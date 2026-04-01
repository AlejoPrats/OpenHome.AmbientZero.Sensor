#pragma once

#include <cstdio>
#include <cstring>
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"

/**
 * @brief Bounded, non‑blocking WiFi scanner using the CYW43 raw API.
 *
 * This class performs multi‑pass WiFi scans without blocking the main
 * loop. It uses an async worker to schedule scan passes and collects
 * only the minimal information required for the provisioning portal.
 *
 * The scanner:
 *  - performs a fixed number of passes
 *  - filters out hidden SSIDs
 *  - stores results in a static, zero‑allocation array
 *  - marks completion via isFinished()
 *
 * @note All state is static. Only one scan operation may run at a time.
 */
class WifiScan {
public:
    /**
     * @brief Begins a WiFi scan consisting of the given number of passes.
     *
     * If a scan is already in progress, this call is ignored.
     *
     * @param scans  Number of scan passes to perform (default: 1).
     */
    static void start(int scans = 1);

    /**
     * @brief Advances the scan state machine.
     *
     * Must be called periodically from the main loop to allow the async
     * worker and scan callbacks to progress.
     */
    static void poll();

    /**
     * @brief Minimal information about a discovered WiFi network.
     *
     * SSID is always null‑terminated and never empty (hidden networks
     * are filtered out before insertion).
     */
    struct NetworkInfo {
        char ssid[33];   // fixed-size, zero-allocation
        bool secure;
    };

    static constexpr int MAX_NETWORKS = 32;
    static constexpr int SCAN_TIMEOUT_MS = 500;

    /**
     * @brief Array of discovered networks.
     *
     * Populated incrementally by scanCallback(). Contains only visible
     * SSIDs and never exceeds MAX_NETWORKS.
     */
    static NetworkInfo networks[MAX_NETWORKS];

    /**
     * @brief Number of valid entries in networks[].
     */
    static int networkCount;

    /**
     * @brief Returns true when all scan passes have completed.
     */
    static bool isFinished();

private:
    static bool scanInProgress;
    static absolute_time_t scanDeadline;

    // Async worker used to schedule scan passes
    static async_at_time_worker_t worker;
    static void workerFn(async_context_t* context, async_at_time_worker_t* w);

    // CYW43 scan result callback
    static int scanCallback(void* env, const cyw43_ev_scan_result_t* result);

    // Marks the scan as complete and finalizes results
    static void finishScan();
};
