#pragma once
#include <cstdint>

/**
 * @brief Metadata describing a firmware update.
 *
 * Contains the minimal fields required to validate an OTA payload:
 *  - version:   numeric version identifier
 *  - size:      expected firmware size in bytes
 *  - checksum:  CRC32 or similar integrity check
 */
#pragma pack(push, 1)
struct OtaManifest {
    char version[32];   // semantic version string
    uint32_t size;      // firmware size in bytes
    uint32_t checksum;  // CRC32 or whatever you're using
};
#pragma pack(pop)