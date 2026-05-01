#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>

// Must match OtaManifest exactly
struct OtaManifest {
    char     version[32];   // semantic version string
    uint32_t size;          // firmware size in bytes
    uint32_t checksum;      // CRC32
};

// Same CRC32 as in ota_service.cpp
uint32_t simple_crc32(const uint8_t* data, uint32_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: gen_manifest <firmware.bin> <manifest.bin> <version_string>\n");
        return 1;
    }

    const char* fw_path       = argv[1];
    const char* manifest_path = argv[2];
    const char* version_str   = argv[3];

    // Load firmware
    FILE* f = fopen(fw_path, "rb");
    if (!f) {
        printf("Error: cannot open firmware file: %s\n", fw_path);
        return 2;
    }

    fseek(f, 0, SEEK_END);
    uint32_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::vector<uint8_t> buf(size);
    fread(buf.data(), 1, size, f);
    fclose(f);

    // Compute checksum
    uint32_t checksum = simple_crc32(buf.data(), size);

    // Fill manifest
    OtaManifest m{};
    // Copy version string safely
    strncpy(m.version, version_str, sizeof(m.version));
    m.version[sizeof(m.version) - 1] = '\0'; // ensure null termination

    m.size = size;
    m.checksum = checksum;

    // Write manifest.bin
    FILE* out = fopen(manifest_path, "wb");
    if (!out) {
        printf("Error: cannot write manifest file: %s\n", manifest_path);
        return 3;
    }

    fwrite(&m, sizeof(m), 1, out);
    fclose(out);

    printf("Manifest generated:\n");
    printf("  version  = %s\n", m.version);
    printf("  size     = %u bytes\n", size);
    printf("  checksum = 0x%08X\n", checksum);

    return 0;
}
