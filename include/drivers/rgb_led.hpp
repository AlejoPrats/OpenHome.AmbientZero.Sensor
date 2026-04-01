#pragma once
#include <cstdint>

/**
 * @brief High‑level LED signaling modes.
 *
 * These modes define the user‑visible patterns used throughout the
 * system. Timing and pattern sequencing are implemented inside the
 * RGBLed driver.
 */
enum LedMode
{
    LED_OK,
    LED_ERROR,
    LED_CAPTIVE,
    LED_SIGNALING,
    LED_BLINK_ONCE,
    LED_BLINK_TWICE,
    LED_OFF
};

class RGBLed
{
public:
    /**
     * @brief Non‑blocking RGB LED driver with pattern support.
     *
     * This class controls a 3‑pin RGB LED and provides both blocking and
     * non‑blocking pattern execution. Non‑blocking patterns rely on poll()
     * being called periodically to advance the sequence.
     *
     * The driver performs no dynamic allocation and maintains all state
     * internally.
     */
    RGBLed(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Initializes the GPIO pins and prepares the LED for use.
     *
     * Must be called once before any mode is set.
     */
    void init();

    /**
     * @brief Sets a signaling mode and returns immediately.
     *
     * This function configures the LED to begin a non‑blocking pattern.
     * The pattern progresses only when poll() is called.
     *
     * @param m  The desired LED mode.
     */
    void set_mode(LedMode m);

    /**
     * @brief Runs a signaling mode to completion before returning.
     *
     * Executes the full LED pattern in a blocking manner. Intended for
     * rare cases where synchronous signaling is acceptable.
     *
     * @param m  The desired LED mode.
     */
    void set_mode_blocking(LedMode m);

    /**
     * @brief Advances non‑blocking LED patterns.
     *
     * Must be called periodically (e.g., inside the main loop) to
     * update timing and step through multi‑phase patterns.
     */
    void poll();

private:
    /**
     * @brief Sets the LED to a specific RGB value.
     *
     * Low‑level helper used by pattern logic.
     */
    void set_rgb(uint8_t r, uint8_t g, uint8_t b);

    // state for non-blocking patterns
    LedMode current_mode = LedMode::LED_OFF;
    uint32_t last_update_ms = 0;
    int step = 0;

    uint8_t pin_r, pin_g, pin_b;
};

/**
 * @brief Global LED instance used by the application.
 *
 * Declared here for convenience; defined in a corresponding .cpp file.
 */
extern RGBLed led;