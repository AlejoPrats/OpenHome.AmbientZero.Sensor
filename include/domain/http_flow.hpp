#pragma once
#include "drivers/rgb_led.hpp"
#include "config/device_config.hpp"

/**
 * @brief Executes the full measurement‑upload workflow.
 *
 * This function performs the complete HTTP transmission sequence:
 * - formats the measurement payload (temperature, humidity, battery)
 * - connects to WiFi using the provided configuration
 * - sends the HTTP request to the backend
 * - processes the server response
 * - drives LED feedback depending on signaling mode
 *
 * @param temperature   Latest temperature reading in °C.
 * @param humidity      Latest humidity reading in %RH.
 * @param battery_raw   Raw ADC value representing battery level.
 * @param isSignaling   When true, LED feedback is enabled during the flow.
 * @param led           Reference to the RGB LED driver for signaling.
 * @param cfg           Device configuration containing WiFi and API settings.
 *
 * @return true if the measurement was successfully transmitted and a valid
 *         server response was received. Returns false on any failure
 *         (WiFi, HTTP, formatting, or server error).
 *
 * @note This function is synchronous at the flow level but relies on
 *       non‑blocking subsystems internally. It does not sleep or reboot.
 */
bool send_measurement_flow(
    float temperature,
    float humidity,
    uint16_t battery_raw,
    bool isSignaling,
    RGBLed &led,
    const DeviceConfig &cfg);
