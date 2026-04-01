#pragma once
#include <stdint.h>

enum class ButtonStatus
{
    IDLE,
    PRESSED,
    RELEASED
};

enum class ButtonAction
{
    NONE,
    NORMAL_BOOT,
    SIGNAL_PRESS,
    PROVISION_PRESS
};

/**
 * @brief Polls the button state during boot and determines user intent.
 *
 * This function must be called repeatedly during the boot window.
 * It debounces the button, tracks press/release transitions, and
 * interprets them as high‑level actions such as:
 * - NORMAL_BOOT
 * - SIGNAL_PRESS
 * - PROVISION_PRESS
 *
 * @return ButtonAction The interpreted action based on the current
 *         button state and timing rules. Returns NONE when no action
 *         has been detected.
 *
 * @note This function performs no blocking delays. It relies on
 *       repeated polling to accumulate timing information.
 */
ButtonAction button_poll();

/**
 * @brief Returns the current low‑level button status.
 *
 * This function exposes the raw debounced state of the button:
 * - IDLE
 * - PRESSED
 * - RELEASED
 *
 * It does not interpret timing or translate the state into actions.
 * That logic is handled by button_poll().
 *
 * @return ButtonStatus The current debounced button state.
 */
ButtonStatus button_status();