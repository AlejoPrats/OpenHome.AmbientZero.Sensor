#include "drivers/flash_layout.hpp"
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"

// Linker-provided boundaries of the firmware image in flash.
extern "C" {
    extern const uint8_t __flash_binary_start;
    extern const uint8_t __flash_binary_end;
}

uint32_t __not_in_flash_func(FlashLayoutService::flash_size())
{
    return PICO_FLASH_SIZE_BYTES;
}

uint32_t __not_in_flash_func(FlashLayoutService::firmware_end_offset())
{
    return (uint32_t)&__flash_binary_end - XIP_BASE;
}

uint32_t __not_in_flash_func(FlashLayoutService::main_region_offset())
{
    return PICO_FLASH_TARGET_OFFSET;
}

uint32_t __not_in_flash_func(FlashLayoutService::ota_region_offset())
{
    return flash_size() / 2;
}

uint32_t __not_in_flash_func(FlashLayoutService::main_region_size())
{
    return ota_region_offset() - main_region_offset();
}

uint32_t __not_in_flash_func(FlashLayoutService::ota_region_size())
{
    return flash_size() - ota_region_offset();
}

uint32_t __not_in_flash_func(FlashLayoutService::config_region_offset())
{
    // Place config in the last 4K before the OTA region.
    return ota_region_offset() - FLASH_SECTOR_SIZE;
}

uint32_t __not_in_flash_func(FlashLayoutService::config_region_size())
{
    return FLASH_SECTOR_SIZE; // always 4K
}