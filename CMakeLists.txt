# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# ====================================================================================
# VSCode Pico Extension Boilerplate (must be FIRST)
# ====================================================================================
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(ambientzero_sensor C CXX ASM)

# Enable C++ features
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

pico_sdk_init()

# ====================================================================================
# VERSION HANDLING
# ====================================================================================
set(VERSION "0.0.0-dev" CACHE STRING "Firmware version string")

if("${VERSION}" STREQUAL "")
    message(FATAL_ERROR "VERSION cannot be empty. Provide -DVERSION=x.y.z")
endif()

message(STATUS "Firmware version: ${VERSION}")

# Generate version_bootstrap.hpp
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/include/config/templates/version_bootstrap.hpp.in
    ${CMAKE_BINARY_DIR}/generated/version_bootstrap.hpp
    @ONLY
)

include_directories(${CMAKE_BINARY_DIR}/generated)

# ====================================================================================
# SOURCE COLLECTION
# ====================================================================================

# Drivers
file(GLOB_RECURSE DRIVER_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/drivers/*.cpp"
)

# Services
file(GLOB_RECURSE SERVICE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*.c"
)

# Domain logic (includes OTA)
file(GLOB_RECURSE DOMAIN_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/domain/*.cpp"
)

# Protocols
file(GLOB_RECURSE PROTOCOL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/protocol/*.cpp"
)

# System
file(GLOB_RECURSE SYSTEM_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/system/*.cpp"
)

# Config
file(GLOB_RECURSE CONFIG_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/config/*.cpp"
)

# ====================================================================================
# EXECUTABLE TARGET
# ====================================================================================

add_executable(ambientzero_sensor
    src/app/main.cpp
    src/app/app_main.cpp

    ${DRIVER_SOURCES}
    ${SERVICE_SOURCES}
    ${DOMAIN_SOURCES}
    ${PROTOCOL_SOURCES}
    ${SYSTEM_SOURCES}
    ${CONFIG_SOURCES}
)

# ====================================================================================
# FLASH LAYOUT
# ====================================================================================

# Reserve flash space for settings
#target_link_options(ambientzero_sensor PRIVATE
#    -Wl,-T${CMAKE_CURRENT_LIST_DIR}/flash_config.ld
#    -Wl,--undefined=__flash_config_start__
#)

# Place application at 0x1000 (boot2 occupies sector 0)
set(PICO_FLASH_TARGET_OFFSET 0x1000)
target_compile_definitions(ambientzero_sensor PRIVATE
    PICO_FLASH_TARGET_OFFSET=${PICO_FLASH_TARGET_OFFSET}
)

# ====================================================================================
# COMPILE DEFINITIONS
# ====================================================================================

target_compile_definitions(ambientzero_sensor PRIVATE
    ENABLE_DEBUG_LIGHTS=0

    PIN_AHT_SDA=16
    PIN_AHT_SCL=17

    PIN_LED_R=2
    PIN_LED_G=1
    PIN_LED_B=0

    PIN_BATTERY_ADC=26
    PIN_OCTOCOUPLER=14
    PIN_SIGNAL_BUTTON=15

    NODE_IP="10.0.0.1"

    HTTP_API_PORT=80
    HTTP_API_PATH="/AmbientTemperature"

    OTA_FIRMWARE_PATH="/ota/firmware.bin"
    OTA_MANIFEST_PATH="/ota/manifest.bin"
    OTA_HTTP_PORT=80
    OTA_PAGE_SIZE=256

    MAX_WATCHDOG_CHUNK=8000

    AP_SSID="AmbientZero-Sensor"
    AP_PASSWORD=""

    DEEP_SLEEP_MS=1200000
)

# ====================================================================================
# INCLUDE DIRECTORIES
# ====================================================================================

target_include_directories(ambientzero_sensor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src/config
)

# ====================================================================================
# LIBRARIES
# ====================================================================================

target_link_libraries(ambientzero_sensor
    pico_stdlib
    pico_runtime
    pico_multicore

    hardware_i2c
    hardware_pwm
    hardware_adc
    hardware_rtc
    hardware_timer
    hardware_clocks
    hardware_sync
    hardware_watchdog
    hardware_spi

    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
)

pico_enable_stdio_uart(ambientzero_sensor 0)
pico_enable_stdio_usb(ambientzero_sensor 1)

pico_set_program_name(ambientzero_sensor "ambientzero_sensor")
pico_set_program_version(ambientzero_sensor ${VERSION})

pico_add_extra_outputs(ambientzero_sensor)

# ====================================================================================
# HOST TOOLS (gen_manifest)
# ====================================================================================

add_custom_target(gen_manifest_tool ALL
    COMMAND g++ "${CMAKE_CURRENT_LIST_DIR}/src/tools/gen_manifest.cpp"
            -o "${CMAKE_BINARY_DIR}/gen_manifest"
    BYPRODUCTS "${CMAKE_BINARY_DIR}/gen_manifest"
    COMMENT "Building host-side gen_manifest tool"
)

add_dependencies(ambientzero_sensor gen_manifest_tool)

# ====================================================================================
# POST-BUILD: OTA PAYLOAD + MANIFEST
# ====================================================================================


# Generate manifest
add_custom_command(TARGET ambientzero_sensor POST_BUILD
    COMMAND "${CMAKE_BINARY_DIR}/gen_manifest"
        "$<TARGET_FILE_DIR:ambientzero_sensor>/ambientzero_sensor.bin"
        "$<TARGET_FILE_DIR:ambientzero_sensor>/manifest.bin"
        "${VERSION}"
    COMMENT "Generating OTA manifest.bin with version ${VERSION}"
)
