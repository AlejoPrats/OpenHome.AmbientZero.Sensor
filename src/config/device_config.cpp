#include "config/device_config.hpp"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>
#include <stdio.h>

// ---------------------------------------------------------
// Flash storage region (allocated by linker)
// ---------------------------------------------------------

// This array is placed in the .flash_config section defined in flash_config.ld
// The linker guarantees this region exists in flash and is safe to read/write.
__attribute__((section(".flash_config")))
uint8_t flash_storage[FLASH_SECTOR_SIZE];

// Pointer to the config region in flash
static const uint8_t* flash_ptr = flash_storage;

// ---------------------------------------------------------
// CRC32 (simple, robust, no dependencies)
// ---------------------------------------------------------
static uint32_t crc32(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    while (len--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return ~crc;
}

// ---------------------------------------------------------
// Detect if flash region is fully erased (all 0xFF)
// ---------------------------------------------------------
bool is_flash_erased(const DeviceConfig* stored) {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(stored);

    for (size_t i = 0; i < sizeof(DeviceConfig); i++) {
        if (ptr[i] != 0xFF) {
            return false; // Not erased
        }
    }
    return true; // Entire struct is erased
}

// ---------------------------------------------------------
// Load config from flash
// ---------------------------------------------------------
bool load_config(DeviceConfig& out) {
    const DeviceConfig* stored = (const DeviceConfig*)flash_ptr;

    // Case 1: Flash is erased → create empty config and return true
    if (is_flash_erased(stored)) {
        memset(&out, 0, sizeof(out));
        out.crc = crc32((uint8_t*)&out, sizeof(DeviceConfig) - sizeof(uint32_t));
        save_config(out);
        return true;
    }

    // Case 2: Flash is not erased → validate CRC
    uint32_t expected = crc32((const uint8_t*)stored,
                              sizeof(DeviceConfig) - sizeof(uint32_t));

    if (expected != stored->crc) {
        return false; // CRC invalid → caller decides what to do
    }

    // Case 3: Valid config
    out = *stored;
    return true;
}

// ---------------------------------------------------------
// Save config to flash
// ---------------------------------------------------------
void save_config(const DeviceConfig& cfg) {
    DeviceConfig temp = cfg;

    // Compute CRC
    temp.crc = crc32((const uint8_t*)&temp,
                     sizeof(DeviceConfig) - sizeof(uint32_t));

    // Prepare a 256-byte page buffer
    uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0xFF, sizeof(page));
    memcpy(page, &temp, sizeof(DeviceConfig));

    // Flash write must be atomic
    uint32_t ints = save_and_disable_interrupts();

    // Erase and write the sector where flash_storage lives
    flash_range_erase((uintptr_t)flash_storage - XIP_BASE, FLASH_SECTOR_SIZE);
    flash_range_program((uintptr_t)flash_storage - XIP_BASE, page, FLASH_PAGE_SIZE);

    restore_interrupts(ints);
}
