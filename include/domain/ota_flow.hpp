#pragma once
#include "domain/ota_flow.hpp"
#include <cstdint>

enum class OtaResult {
    Running,
    ReadyToApply,
    Failed
};

void ota_setup(const char* server_ip);
OtaResult ota_loop();
uint32_t ota_get_firmware_size();
