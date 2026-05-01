#pragma once

#include <cstdint>
#include "drivers/sleeper.hpp"

/**
 * @brief Handles deep‑sleep entry and low‑power transitions.
 *
 * This class coordinates the steps required to safely enter deep sleep
 * on the RP2040, including:
 *  - preparing the CYW43 WiFi subsystem (if initialized)
 *  - switching the system clock to ROSC
 *  - reducing clock frequencies for minimal power draw
 *  - performing a controlled reboot into sleep mode
 *
 * The flow is split into:
 *  - continueDeepSleep(): called at boot to resume a pending sleep
 *  - requestDeepSleep(): called at the end of a measurement flow
 *  - rebootForSleep():   performs the actual reboot into sleep entry
 *
 * @note This class does not decide *when* to sleep. It only executes
 *       the low‑power transition once requested.
 */
class PowerManager
{
public:
    /**
     * @brief Resumes a pending deep‑sleep sequence after reboot.
     *
     * Called at the start of main(). If the previous flow requested
     * deep sleep, this function completes the transition by preparing
     * clocks and entering low‑power mode.
     *
     * @param isWifiInitialized  True if CYW43 was already initialized.
     */
    void continue_deep_sleep(bool isWifiInitialized);

    /**
     * @brief Requests deep sleep after the current flow completes.
     *
     * Stores the desired sleep duration and triggers a reboot so the
     * device can enter low‑power mode from a clean state.
     *
     * @param ms  Duration of deep sleep in milliseconds.
     */
    void request_deep_sleep(uint64_t ms);

    /**
     * @brief Performs a controlled reboot into the deep‑sleep entry path.
     *
     * Called after requestDeepSleep(). Ensures that the reboot occurs
     * only after all pending operations (e.g., network sends) have
     * completed.
     */
    void reboot_for_sleep();

    /**
     * @brief Triggers a full system reset from RAM.
     *
     * This function performs a Cortex‑M0+ system reset by writing the
     * SYSRESETREQ bit in the Application Interrupt and Reset Control
     * Register (AIRCR). It is marked with @ref __not_in_flash_func so the
     * implementation is placed entirely in RAM, ensuring it remains
     * executable even after flash has been erased or reprogrammed during
     * OTA updates.
     *
     * This function does not return. After requesting the reset, it enters
     * an infinite loop while waiting for the hardware reset to occur.
     *
     * @note This reset method is safe to call after flash operations,
     *       including rewriting the main application region.
     *
     * @warning Must only be called when the system is in a flash‑safe
     *          state (interrupts disabled, Wi‑Fi chip powered down, and
     *          no code executing from flash).
     */
    static void ram_system_reset();

    /**
     * @brief Prepares the system for safe flash operations.
     *
     * Ensures the hardware is placed into a stable state before performing
     * flash writes. This includes shutting down the Wi‑Fi subsystem (if active),
     * power‑cycling the CYW43 chip to guarantee a clean post‑reset state, and
     * disabling all interrupts to prevent execution from returning to flash.
     *
     * @param wifiInitialized Indicates whether the Wi‑Fi subsystem is currently active.
     */
    void enter_flash_safe_state(bool wifiInitialized);

private:
    /**
     * @brief Prepares the WiFi subsystem for low‑power entry.
     *
     * Ensures the CYW43 is cleanly shut down before clocks are reduced.
     */
    void prepareWifiForSleep();

    /**
     * @brief Switches the system clock source to ROSC.
     *
     * This is the first step in reducing power consumption before
     * lowering clock frequencies.
     */
    void switchToROSC();

    /**
     * @brief Reduces system clocks after switching to ROSC.
     *
     * Applies additional clock reductions to minimize power draw during
     * deep sleep.
     */
    void reduceClocksAfterROSC();

    /**
     * @brief Enters low‑power mode after all preparation steps.
     *
     * Handles the final transition into deep sleep, including any
     * required delays or hardware sequencing.
     */
    void enterLowPower(bool isWifiInitialized);

    /**
     * @brief Gracefully shuts down the Wi‑Fi and networking stack.
     *
     * Disconnects from the network, transitions the Wi‑Fi firmware into a
     * low‑power state, flushes pending lwIP/SDIO activity, and fully deinitializes
     * the CYW43 subsystem. This ensures no SDIO or networking activity interferes
     * with flash operations.
     */
    void shutdownWifiStack();

    /**
     * @brief Fully resets the CYW43 Wi‑Fi chip via WL_REG_ON.
     *
     * Toggles the WL_REG_ON control pin (WIFI_REG_ON_PIN) to force a complete
     * hardware reset of the CYW43 module. This is required because software
     * resets (e.g., watchdog resets) do not reset the Wi‑Fi chip, which can
     * otherwise remain in an undefined state and block system boot.
     */
    void powerCycleWifiChip();

    Sleeper sleeper;
};
