#pragma once

#include <stdint.h>
#include <stddef.h>

// A small, fixed-size configuration block stored in flash.
// CRC ensures integrity and avoids loading corrupted data.
struct DeviceConfig {
    char id[64];
    char ssid[32];
    char password[64];
    uint32_t crc;
};

// Loads the configuration from flash.
// Returns true if valid, false if missing or corrupted.
bool load_config(DeviceConfig& out);

// Saves the configuration to flash (atomic sector write).
void save_config(const DeviceConfig& cfg);
