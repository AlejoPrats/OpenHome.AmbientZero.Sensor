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
    void continueDeepSleep(bool isWifiInitialized);

    /**
     * @brief Requests deep sleep after the current flow completes.
     *
     * Stores the desired sleep duration and triggers a reboot so the
     * device can enter low‑power mode from a clean state.
     *
     * @param ms  Duration of deep sleep in milliseconds.
     */
    void requestDeepSleep(uint64_t ms);

    /**
     * @brief Performs a controlled reboot into the deep‑sleep entry path.
     *
     * Called after requestDeepSleep(). Ensures that the reboot occurs
     * only after all pending operations (e.g., network sends) have
     * completed.
     */
    void rebootForSleep();

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

    Sleeper sleeper;
};
