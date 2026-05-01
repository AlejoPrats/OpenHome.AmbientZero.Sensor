#include "drivers/flash_writer.hpp"
#include "drivers/flash_layout.hpp"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"
#include "hardware/regs/addressmap.h"

#include "system/power_manager.hpp"

#include <cstdint>

// -----------------------------------------------------------------------------
// OTA staging: eraseRegion (uses FlashLayoutService, runs in normal flash context)
// -----------------------------------------------------------------------------

bool __not_in_flash_func(FlashWriter::erase_region)()
{
    uint32_t base = FlashLayoutService::ota_region_offset();
    uint32_t size = FlashLayoutService::ota_region_size();

    if (base == 0 || size == 0)
    {
        return false;
    }

    constexpr uint32_t SECTOR_SIZE = 4096;
    uint32_t count = size / SECTOR_SIZE;

    uint32_t ints = save_and_disable_interrupts();
    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t addr = base + i * SECTOR_SIZE;
        flash_range_erase(addr, SECTOR_SIZE);
    }
    restore_interrupts(ints);

    return true;
}

// -----------------------------------------------------------------------------
// OTA staging: writeChunk (uses FlashLayoutService, runs in normal flash context)
// -----------------------------------------------------------------------------

bool __not_in_flash_func(FlashWriter::write_chunk)(
    uint32_t offset,
    const uint8_t *data,
    uint32_t len)
{
    static uint32_t base_offset = 0xFFFFFFFF;

    uint32_t base = FlashLayoutService::ota_region_offset();
    uint32_t size = FlashLayoutService::ota_region_size();

    if (base == 0 || size == 0)
    {
        return false;
    }

    if (base_offset == 0xFFFFFFFF)
    {
        base_offset = offset;
    }

    uint32_t normalized_offset = offset - base_offset;

    if (normalized_offset + len > size)
    {
        return false;
    }

    constexpr uint32_t PAGE_SIZE = 256;
    if ((normalized_offset % PAGE_SIZE) != 0 || (len % PAGE_SIZE) != 0)
    {
        return false;
    }

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(base + normalized_offset, data, len);
    restore_interrupts(ints);

    return true;
}

// -----------------------------------------------------------------------------
// RAM-only helpers for applyOtaToMain
// -----------------------------------------------------------------------------

static void *__not_in_flash_func(ram_memcpy)(void *dst, const void *src, uint32_t len)
{
    auto *d = static_cast<uint8_t *>(dst);
    auto *s = static_cast<const uint8_t *>(src);

    while (len--)
    {
        *d++ = *s++;
    }
    return dst;
}

static void *__not_in_flash_func(ram_memset)(void *dst, int value, uint32_t len)
{
    auto *d = static_cast<uint8_t *>(dst);
    const uint8_t v = static_cast<uint8_t>(value);

    while (len--)
    {
        *d++ = v;
    }
    return dst;
}

// -----------------------------------------------------------------------------
// OTA apply logic
// -----------------------------------------------------------------------------

extern "C" void __not_in_flash_func(FlashWriter_applyOtaToMain_C)(uint32_t firmware_size)
{
    __asm volatile("cpsid i" ::: "memory");

    const uint32_t MAIN_OFFSET = 0x00000000;
    const uint32_t OTA_OFFSET = 0x00100000; // OTA flash offset

    const uint32_t MAIN_BASE = MAIN_OFFSET;          // for flash_range_*
    const uint32_t OTA_BASE = XIP_BASE + OTA_OFFSET; // CPU address

    const uint32_t SECTOR_SIZE = FLASH_SECTOR_SIZE;
    const uint32_t PAGE_SIZE = FLASH_PAGE_SIZE;

    uint8_t buffer[FLASH_SECTOR_SIZE];
    uint32_t offset = 0;

    while (offset < firmware_size)
    {
        uint32_t chunk = firmware_size - offset;
        if (chunk > SECTOR_SIZE)
            chunk = SECTOR_SIZE;

        // READ via XIP address
        ram_memcpy(buffer, reinterpret_cast<const void *>(OTA_BASE + offset), chunk);

        // ERASE via flash offset
        flash_range_erase(MAIN_BASE + offset, SECTOR_SIZE);

        uint32_t prog_size = (chunk + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        if (prog_size > SECTOR_SIZE)
            prog_size = SECTOR_SIZE;

        if (prog_size > chunk)
            ram_memset(buffer + chunk, 0xFF, prog_size - chunk);

        // PROGRAM via flash offset
        flash_range_program(MAIN_BASE + offset, buffer, prog_size);

        offset += chunk;
    }

    PowerManager::ram_system_reset();

    while (true)
    {
        tight_loop_contents();
    }
}

// -----------------------------------------------------------------------------
// C++ wrapper (never returns)
// -----------------------------------------------------------------------------

[[noreturn]] void FlashWriter::apply_ota_to_main(uint32_t firmwareSize)
{
    FlashWriter_applyOtaToMain_C(firmwareSize);
    // This point should never be reached.
    __builtin_unreachable();
}