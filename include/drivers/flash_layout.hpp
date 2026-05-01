#pragma once

#include <cstdint>

/**
 * @brief Describes the flash memory layout used by the firmware and OTA system.
 *
 * FlashLayoutService provides static helpers that define the boundaries of the
 * main application region, the OTA staging region, and the persistent config
 * region within external flash. These values are derived from the configured
 * flash size and do not depend on linker‑placed sections.
 *
 * All offsets returned by this service are flash offsets (not XIP addresses).
 */
class FlashLayoutService
{
public:
    /**
     * @brief Returns the total size of external flash in bytes.
     *
     * This value is defined by the Pico SDK configuration
     * (PICO_FLASH_SIZE_BYTES) and corresponds to the actual size of the flash
     * chip connected to the RP2040.
     */
    static uint32_t flash_size();

    /**
     * @brief Returns the flash offset immediately after the linked firmware image.
     *
     * Computed from the linker-provided symbol __flash_binary_end and converted
     * from an XIP address into a flash offset. Represents the end of the main
     * firmware image as placed by the linker.
     */
    static uint32_t firmware_end_offset();

    /**
     * @brief Returns the flash offset where the main application region begins.
     *
     * This corresponds to PICO_FLASH_TARGET_OFFSET, which is determined by the
     * linker script and typically accounts for the bootloader and metadata area.
     */
    static uint32_t main_region_offset();

    /**
     * @brief Returns the flash offset where the OTA staging region begins.
     *
     * The OTA region is placed in the upper half of flash to ensure it never
     * overlaps with the main application region. The OTA image must fit entirely
     * within this region before being applied.
     */
    static uint32_t ota_region_offset();

    /**
     * @brief Returns the size in bytes of the main application region.
     *
     * This is the space between the start of the main region and the start of
     * the OTA region. The main firmware must fit entirely within this range.
     */
    static uint32_t main_region_size();

    /**
     * @brief Returns the size in bytes of the OTA staging region.
     *
     * This is the space from the OTA region offset to the end of flash. The OTA
     * image is downloaded into this region before being applied.
     */
    static uint32_t ota_region_size();

    /**
     * @brief Returns the flash offset where the persistent config sector begins.
     *
     * The config region is placed immediately before the OTA region and occupies
     * exactly one flash sector (4 KiB). This ensures it never overlaps with the
     * main firmware or the OTA staging area.
     */
    static uint32_t config_region_offset();

    /**
     * @brief Returns the size of the persistent config region in bytes.
     *
     * The configuration block occupies exactly one flash sector (4 KiB), which
     * is the minimum erasable unit of the RP2040 external flash.
     */
    static uint32_t config_region_size();
};
