#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Persistent device configuration stored in flash.
 *
 * This structure contains all user‑provided settings required for
 * normal operation. It is written as a fixed‑size block and protected
 * by a CRC to ensure integrity and prevent loading corrupted data.
 *
 * Fields:
 * - id:        Unique device identifier (null‑terminated)
 * - ssid:      WiFi network name
 * - password:  WiFi password
 * - crc:       CRC32 of all preceding fields
 *
 * @note The structure layout must remain stable. Any changes require
 *       a versioning or migration strategy.
 */
struct DeviceConfig
{
    char id[64];
    char ssid[32];
    char password[64];
    uint32_t crc;
};

/**
 * @brief Loads the configuration block from flash.
 *
 * Reads the stored DeviceConfig, verifies its CRC, and returns the
 * result to the caller.
 *
 * @param out  Reference to a DeviceConfig structure to populate.
 *
 * @return true if a valid configuration was found and passed CRC
 *         validation. Returns false if the region is erased,
 *         uninitialized, or corrupted.
 */
bool load_config(DeviceConfig &out);

/**
 * @brief Saves the configuration block to flash.
 *
 * Performs an atomic sector erase + write of the DeviceConfig
 * structure, including CRC calculation.
 *
 * @param cfg  Configuration to persist.
 *
 * @note This operation rewrites an entire flash sector. It should be
 *       used sparingly to avoid unnecessary wear.
 */
void save_config(const DeviceConfig &cfg);

/**
 * @brief Checks whether the stored flash region is fully erased.
 *
 * Determines if the underlying flash memory contains only 0xFF bytes,
 * which indicates an uninitialized or factory‑reset state.
 *
 * @param stored  Pointer to the raw flash region mapped as DeviceConfig.
 *
 * @return true if the region is fully erased, false otherwise.
 */
bool is_flash_erased(const DeviceConfig *stored);
