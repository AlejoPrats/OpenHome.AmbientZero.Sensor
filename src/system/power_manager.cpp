#include "system/power_manager.hpp"
#include "services/scratch_handler.hpp"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "pico/cyw43_arch.h"
#include "cyw43.h"

void PowerManager::reduceClocksAfterROSC()
{
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

void PowerManager::request_deep_sleep(uint64_t total_ms)
{

    if (get_boot_flag() == BootFlag::OTA_PENDING)
    {
        ScratchHandler::set(ScratchHandler::CHUNKS, 1);
        ScratchHandler::set(ScratchHandler::REMAINDER, 0);
        ScratchHandler::set(ScratchHandler::LAST_CHUNK, 0);
        return;
    }

    uint32_t chunks = total_ms / MAX_WATCHDOG_CHUNK;
    uint32_t remainder = total_ms % MAX_WATCHDOG_CHUNK;

    ScratchHandler::set(ScratchHandler::CHUNKS, chunks);
    ScratchHandler::set(ScratchHandler::REMAINDER, remainder);
    ScratchHandler::set(ScratchHandler::LAST_CHUNK, 0);
    ScratchHandler::set(ScratchHandler::BOOT_FLAG, 1);
}

void PowerManager::continue_deep_sleep(bool isWifiInitialized)
{
    BootFlag mode = get_boot_flag();

    if (mode == BootFlag::NONE)
    {
        return; // normal execution
    }

    uint32_t chunks = ScratchHandler::get(ScratchHandler::CHUNKS);
    uint32_t remainder = ScratchHandler::get(ScratchHandler::REMAINDER);
    uint32_t lastChunk = ScratchHandler::get(ScratchHandler::LAST_CHUNK);

    uint32_t sleep_ms = 0;

    if (lastChunk < chunks)
    {
        lastChunk++;
        ScratchHandler::set(ScratchHandler::LAST_CHUNK, lastChunk);
        sleep_ms = MAX_WATCHDOG_CHUNK;
    }
    else
    {
        sleep_ms = remainder;
        if(mode == BootFlag::DEEP_SLEEP)
        {
            set_boot_flag(BootFlag::NONE);
        }
    }

    if (sleep_ms == 0)
    {
        return; // nothing to sleep
    }

    enterLowPower(isWifiInitialized);
    sleeper.sleep_chunk_ms(sleep_ms); // never returns
}

void PowerManager::enterLowPower(bool isWifiInitialized)
{
    if (isWifiInitialized)
    {
        prepareWifiForSleep();
    }
    switchToROSC();          // ROSC switch
    reduceClocksAfterROSC(); // kill PLLs and extra clocks
}

void PowerManager::reboot_for_sleep()
{
    watchdog_reboot(0, 0, 0);
}

void PowerManager::enter_flash_safe_state(bool wifiInitialized)
{
    if (wifiInitialized)
    {
        shutdownWifiStack();
    }

    powerCycleWifiChip();
}

void PowerManager::shutdownWifiStack()
{
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    cyw43_wifi_pm(&cyw43_state, CYW43_PM2_POWERSAVE_MODE);

    for (int i = 0; i < 10; ++i)
    {
        cyw43_arch_poll();
        sleep_ms(50);
    }

    cyw43_arch_deinit();
    sleep_ms(50);
}

void PowerManager::powerCycleWifiChip()
{
    gpio_init(23);
    gpio_set_dir(23, GPIO_OUT);

    gpio_put(23, 0);
    sleep_ms(50);

    gpio_put(23, 1);
    sleep_ms(50);
}

void __not_in_flash_func(PowerManager::ram_system_reset)()
{
    // NVIC_SystemReset equivalent: write SYSRESETREQ with key to AIRCR
    volatile uint32_t *AIRCR = (uint32_t *)0xE000ED0C;
    const uint32_t VECTKEY = 0x5FA << 16;
    *AIRCR = VECTKEY | (1u << 2); // SYSRESETREQ

    while (true)
    {
        // wait for reset
    }
}
