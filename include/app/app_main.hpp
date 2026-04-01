#pragma once

/**
 * @brief Executes a single application cycle.
 *
 * This function performs one full iteration of the device's main workflow.
 * It handles sensor reading, network communication, server interaction,
 * and power‑management decisions according to the current device state.
 *
 * @param isSignaling
 *        When true, the cycle runs in "signaling mode", meaning the device
 *        prioritizes user‑visible feedback (LED patterns, button handling)
 *        instead of the normal measurement → upload → sleep sequence.
 *
 * @note This function is intentionally stateless. All persistent state is
 *       stored externally (scratch registers, config storage, services).
 */
void app_main_run_cycle(bool isSignaling);

/**
 * @brief Initiates the provisioning workflow.
 *
 * This function transitions the device into provisioning mode, enabling
 * the WiFi Access Point, DNS hijacking, HTTP server, and captive portal
 * required for the user to configure network credentials.
 *
 * @note Provisioning is always an explicit action. This function must be
 *       called intentionally and is never triggered automatically.
 */
void app_main_start_provisioning();
