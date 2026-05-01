#include "services/ota_service.hpp"
#include "drivers/flash_layout.hpp"
#include "domain/wifi_flow.hpp"
#include <cstring>

bool OtaService::start()
{
    if (state != State::Idle)
        return false;

    HttpDownloadRequest dreq;
    req_.ip = NODE_IP;
    req_.port = OTA_HTTP_PORT;
    req_.path = OTA_MANIFEST_PATH;
    req_.filename = "manifest.bin";
    req_.onChunk = [this](const uint8_t *d, uint32_t l)
    { handleManifestChunk(d, l); };
    req_.onDone = [this](bool ok)
    {             if (!ok)
            {
                state = State::Done;
                this->success = false;
            }
            else if (state == State::DownloadManifest)
            {
                state = State::DownloadFirmware;
            } };

    state = State::DownloadManifest;
    bytesWritten = 0;
    page_staging_len_ = 0;
    firmware_size_ = 0;
    success = false;

    client.download_file(req_);

    return true;
}

void OtaService::poll()
{
    // Required for lwIP progress
    client.poll();

    // ⚠️ Required to avoid stack corruption during OTA.
    printf("OTA state=%d bytesWritten=%u firmware_size=%u\n",
           (int)state, bytesWritten, firmware_size_);

    // ---------------------------------------------------------
    // Start firmware download once manifest is done
    // ---------------------------------------------------------
    if (state == State::DownloadFirmware && bytesWritten == 0 && !isFirmwareDownloading)
    {
        req_.ip = NODE_IP;
        req_.port = OTA_HTTP_PORT;
        req_.path = OTA_FIRMWARE_PATH;
        req_.filename = "firmware.bin";

        req_.onChunk = [this](const uint8_t *d, uint32_t l)
        {
            handleFirmwareChunk(d, l);
        };

        req_.onDone = [this](bool ok)
        {
            if (!ok)
            {
                state = State::Done;
                this->success = false;
            }
            else
            {
                // Flush last partial page
                if (page_staging_len_ > 0)
                {
                    std::memset(page_staging_ + page_staging_len_, 0xFF,
                                OTA_PAGE_SIZE - page_staging_len_);
                    writer.write_chunk(bytesWritten, page_staging_, OTA_PAGE_SIZE);
                    bytesWritten += OTA_PAGE_SIZE;
                    page_staging_len_ = 0;
                }

                state = State::Verify;
            }
        };

        isFirmwareDownloading = true;
        client.download_file(req_);
    }

    // ---------------------------------------------------------
    // Verification stage
    // ---------------------------------------------------------
    if (state == State::Verify)
    {
        success = (bytesWritten >= firmware_size_);
        state = State::Done;
    }
}

bool OtaService::is_finished() const
{
    return state == State::Done;
}

bool OtaService::was_successful() const
{
    return success;
}

void OtaService::handleManifestChunk(const uint8_t *data, uint32_t len)
{
    static uint8_t buffer[sizeof(OtaManifest)];
    static uint32_t received = 0;

    uint32_t copyLen = (len < (sizeof(OtaManifest) - received))
                           ? len
                           : (sizeof(OtaManifest) - received);

    std::memcpy(buffer + received, data, copyLen);
    received += copyLen;

    if (received < sizeof(OtaManifest))
        return;

    std::memcpy(&manifest, buffer, sizeof(OtaManifest));

    /*
    // Future explicit parsing (kept as a reminder)
    std::memcpy(manifest.version, buffer, 32);
    manifest.version[31] = '\0';

    manifest.size = (uint32_t(buffer[32])      ) |
                    (uint32_t(buffer[33]) <<  8) |
                    (uint32_t(buffer[34]) << 16) |
                    (uint32_t(buffer[35]) << 24);

    manifest.checksum = (uint32_t(buffer[36])      ) |
                        (uint32_t(buffer[37]) <<  8) |
                        (uint32_t(buffer[38]) << 16) |
                        (uint32_t(buffer[39]) << 24);
    */

    firmware_size_ = manifest.size;
    received = 0;
}

void OtaService::handleFirmwareChunk(const uint8_t *data, uint32_t len)
{
    if (bytesWritten == 0 && page_staging_len_ == 0)
        writer.erase_region();

    uint32_t offset = 0;

    while (len > 0)
    {
        uint32_t space = OTA_PAGE_SIZE - page_staging_len_;
        uint32_t chunk = (len < space) ? len : space;

        std::memcpy(page_staging_ + page_staging_len_, data + offset, chunk);
        page_staging_len_ += chunk;
        offset += chunk;
        len -= chunk;

        if (page_staging_len_ == OTA_PAGE_SIZE)
        {
            writer.write_chunk(bytesWritten, page_staging_, OTA_PAGE_SIZE);
            bytesWritten += OTA_PAGE_SIZE;
            page_staging_len_ = 0;
        }
    }

    if (bytesWritten + page_staging_len_ >= firmware_size_)
    {
        if (page_staging_len_ > 0)
        {
            std::memset(page_staging_ + page_staging_len_, 0xFF,
                        OTA_PAGE_SIZE - page_staging_len_);
            writer.write_chunk(bytesWritten, page_staging_, OTA_PAGE_SIZE);
            bytesWritten += OTA_PAGE_SIZE;
            page_staging_len_ = 0;
        }

        state = State::Verify;
    }
}
