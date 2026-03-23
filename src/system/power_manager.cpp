#include "power_manager.hpp"
#include "scratch_handler.hpp"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "pico/cyw43_arch.h"
#include "cyw43.h"

void PowerManager::reduceClocksAfterROSC()
{
    // At this point you are already running from ROSC
    // (your existing switchToROSC() has been called)

    // 1. Stop USB clock
    clock_stop(clk_usb);

    // 2. Stop ADC clock
    clock_stop(clk_adc);

    // 3. Disable PLLs – this is what MicroPython also does
    pll_deinit(pll_sys);
    pll_deinit(pll_usb);

    // 4. Optional: stop peripheral clock if you know all peripherals are idle
    // clock_stop(clk_peri);
}

void PowerManager::prepareWifiForSleep()
{
    // 1. Disconnect Wi‑Fi
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    // 2. Put Wi‑Fi firmware into low-power mode
    cyw43_wifi_pm(&cyw43_state, CYW43_PM2_POWERSAVE_MODE);
    // 3. Stop lwIP timers (prevents SDIO activity)
    cyw43_arch_poll(); // flush pending work
}

// ---------------------------------------------------------
void PowerManager::switchToROSC()
{
    clock_configure(
        clk_sys,
        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        6 * MHZ,
        6 * MHZ);
}

void PowerManager::requestDeepSleep(uint64_t total_ms) {
    uint32_t chunks    = total_ms / MAX_WATCHDOG_CHUNK;
    uint32_t remainder = total_ms % MAX_WATCHDOG_CHUNK;

    ScratchHandler::set(ScratchHandler::CHUNKS, chunks);
    ScratchHandler::set(ScratchHandler::REMAINDER, remainder);
    ScratchHandler::set(ScratchHandler::LAST_CHUNK, 0);
    ScratchHandler::set(ScratchHandler::SHOULD_SLEEP, 1);
}

void PowerManager::continueDeepSleep(bool isWifiInitialized) {
    if (ScratchHandler::get(ScratchHandler::SHOULD_SLEEP) != 1) {
        return; // normal execution
    }

    uint32_t chunks     = ScratchHandler::get(ScratchHandler::CHUNKS);
    uint32_t remainder  = ScratchHandler::get(ScratchHandler::REMAINDER);
    uint32_t lastChunk  = ScratchHandler::get(ScratchHandler::LAST_CHUNK);

    uint32_t sleep_ms = 0;

    if (lastChunk < chunks) {
        lastChunk++;
        ScratchHandler::set(ScratchHandler::LAST_CHUNK, lastChunk);
        sleep_ms = MAX_WATCHDOG_CHUNK;
    } else {
        sleep_ms = remainder;
        ScratchHandler::set(ScratchHandler::SHOULD_SLEEP, 0);
    }

    if (sleep_ms == 0) {
        return; // nothing to sleep
    }

    enterLowPower(isWifiInitialized);
    sleeper.sleepChunkMs(sleep_ms); // never returns
}

void PowerManager::enterLowPower(bool isWifiInitialized)
{
    if(isWifiInitialized)
    {
        prepareWifiForSleep();
    }
    switchToROSC();          // your working ROSC switch
    reduceClocksAfterROSC(); // new: kill PLLs and extra clocks

}

void PowerManager::rebootForSleep()
{
    watchdog_reboot(0, 0, 0);
}
