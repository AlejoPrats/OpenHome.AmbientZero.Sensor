#include "device_config.hpp"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>
#include <stdio.h>

#define FLASH_TARGET_OFFSET (2 * 1024 * 1024 - 4096)  // Last 4KB sector

static const uint8_t* flash_ptr = (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);

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
// Load config from flash
// ---------------------------------------------------------
bool load_config(DeviceConfig& out) {
    const DeviceConfig* stored = (const DeviceConfig*)flash_ptr;

    // Flash defaults to 0xFF when erased
    if (stored->id[0] == (char)0xFF)
        return false;

    // Validate CRC
    uint32_t expected = crc32((const uint8_t*)stored,
                              sizeof(DeviceConfig) - sizeof(uint32_t));

    if (expected != stored->crc)
        return false;

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
  
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, page, FLASH_PAGE_SIZE);

    restore_interrupts(ints);
}
