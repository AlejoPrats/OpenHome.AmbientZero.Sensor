#pragma once

#include <cstdint>

/**
 * @brief RAM‑safe flash programming utilities for OTA firmware updates.
 *
 * The FlashWriter class provides the minimal set of operations required to
 * receive a firmware image into the OTA staging region and later apply it to
 * the main application region. All staging operations run from flash, while
 * the final apply step executes entirely from RAM to ensure safe flash erase
 * and program operations without executing code from flash.
 *
 * The OTA workflow is:
 *   1. eraseRegion() — clear the OTA staging area
 *   2. writeChunk()  — stream the new firmware into the OTA region
 *   3. applyOtaToMain() — copy OTA → main and reboot into the new firmware
 *
 * The apply step never returns; after the watchdog reset, execution continues
 * in the newly programmed firmware.
 */
class FlashWriter
{
public:
    /**
     * @brief Erases the entire OTA staging region in flash.
     *
     * Removes all existing data from the OTA region by erasing each sector
     * belonging to it. This prepares the region to receive a new firmware
     * image. Region boundaries are obtained from FlashLayoutService.
     *
     * @return true if the region was successfully erased; false if the layout
     *         is invalid or out of bounds.
     */
    bool erase_region();

    /**
     * @brief Writes a page‑aligned chunk of data into the OTA staging region.
     *
     * Programs a portion of the incoming firmware image into the OTA region.
     * The first call establishes a base offset so subsequent writes can be
     * expressed as a continuous stream. Both the offset and length must be
     * aligned to flash page boundaries. Region boundaries are validated
     * against FlashLayoutService.
     *
     * @param offset Absolute byte offset within the OTA region.
     * @param data   Pointer to the page‑aligned data buffer to write.
     * @param len    Length of the chunk in bytes (must be a multiple of the
     *               flash page size).
     * @return true if the chunk was successfully written; false on alignment
     *         errors or out‑of‑bounds writes.
     */
    bool write_chunk(uint32_t offset, const uint8_t *data, uint32_t len);

    /**
     * @brief Applies the OTA image to the main application region and reboots.
     *
     * Executes entirely from RAM. Copies the firmware image stored in the OTA
     * region into the main application region, sector by sector, using
     * RAM‑resident erase and program operations. After programming completes,
     * the function triggers a watchdog reboot into the newly written firmware.
     *
     * This function never returns. After the watchdog reset, execution begins
     * at the entry point of the new firmware.
     *
     * @param firmwareSize Size in bytes of the firmware image stored in the
     *                     OTA region.
     * @return This function never returns; the return type is preserved for
     *         interface compatibility.
     */
    [[noreturn]] static void apply_ota_to_main(uint32_t firmwareSize);
};
