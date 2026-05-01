#pragma once
#include "config/ota_manifest.hpp"
#include "drivers/flash_writer.hpp"
#include "services/http_client.hpp"

/**
 * @brief High-level OTA orchestrator.
 *
 * Handles:
 *  - downloading the manifest
 *  - downloading the firmware in chunks
 *  - writing to flash
 *  - verifying integrity
 *  - signaling completion
 */
class OtaService
{
public:
    bool start();
    void poll();
    bool is_finished() const;
    bool was_successful() const;
    uint32_t getFirmwareSize() const { return firmware_size_; }

private:
    HttpDownloadRequest req_;
    uint8_t page_staging_[256];
    uint32_t page_staging_len_ = 0;
    bool isFirmwareDownloading = false;

    enum class State
    {
        Idle,
        DownloadManifest,
        DownloadFirmware,
        Verify,
        Done
    };

    State state = State::Idle;
    OtaManifest manifest{};
    FlashWriter writer;
    HttpClient client;

    uint32_t bytesWritten = 0;
    bool success = false;
    uint32_t firmware_size_ = 0;

    void handleManifestChunk(const uint8_t *data, uint32_t len);
    void handleFirmwareChunk(const uint8_t *data, uint32_t len);
};
