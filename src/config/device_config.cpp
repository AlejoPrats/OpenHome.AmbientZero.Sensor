#include "config/device_config.hpp"

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"
#include "drivers/flash_layout.hpp"
#include <string.h>
#include <stdio.h>

// ---------------------------------------------------------
// CRC32
// ---------------------------------------------------------
static uint32_t crc32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    while (len--)
    {
        crc ^= *data++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return ~crc;
}

// ---------------------------------------------------------
// Pointer to config in XIP flash
// ---------------------------------------------------------
static inline const DeviceConfig *config_flash_ptr()
{
    uint32_t offset = FlashLayoutService::config_region_offset();
    return reinterpret_cast<const DeviceConfig *>(XIP_BASE + offset);
}

// ---------------------------------------------------------
// Detect if flash region is fully erased (all 0xFF)
// ---------------------------------------------------------
static bool is_flash_erased(const DeviceConfig *stored)
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(stored);

    for (size_t i = 0; i < sizeof(DeviceConfig); i++)
    {
        if (ptr[i] != 0xFF)
        {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------
// Load config from flash
// ---------------------------------------------------------
bool load_config(DeviceConfig &out)
{
    const DeviceConfig *stored = config_flash_ptr();

    // Case 1: Flash is erased → create empty config and persist it
    if (is_flash_erased(stored))
    {
        memset(&out, 0, sizeof(out));
        out.crc = crc32(reinterpret_cast<const uint8_t *>(&out),
                        sizeof(DeviceConfig) - sizeof(uint32_t));
        save_config(out);
        return true;
    }

    // Case 2: Flash is not erased → validate CRC
    uint32_t expected = crc32(reinterpret_cast<const uint8_t *>(stored),
                              sizeof(DeviceConfig) - sizeof(uint32_t));

    if (expected != stored->crc)
    {
        return false; // CRC invalid
    }

    // Case 3: Valid config
    out = *stored;
    return true;
}

// ---------------------------------------------------------
// Save config to flash
// ---------------------------------------------------------
void save_config(const DeviceConfig &cfg)
{
    DeviceConfig temp = cfg;

    // Compute CRC
    temp.crc = crc32(reinterpret_cast<const uint8_t *>(&temp),
                     sizeof(DeviceConfig) - sizeof(uint32_t));

    // Prepare a 256-byte page buffer
    uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0xFF, sizeof(page));
    memcpy(page, &temp, sizeof(DeviceConfig));

    uint32_t ints = save_and_disable_interrupts();

    uint32_t offset = FlashLayoutService::config_region_offset();

    flash_range_erase(offset, FlashLayoutService::config_region_size());
    flash_range_program(offset, page, FLASH_PAGE_SIZE);

    restore_interrupts(ints);
}

void sync_config_version(const char *firmwareVersion)
{
    DeviceConfig cfg;
    if (!load_config(cfg)) {
        // load_config already initializes defaults and saves them
        load_config(cfg);
    }

    // If empty or different → update
    if (cfg.version[0] == '\0' ||
        strncmp(cfg.version, firmwareVersion, sizeof(cfg.version)) != 0)
    {
        strncpy(cfg.version, firmwareVersion, sizeof(cfg.version));
        cfg.version[sizeof(cfg.version) - 1] = '\0'; // safety
        save_config(cfg);
    }
}
