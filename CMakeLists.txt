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
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT (VSCode Pico extension) ==
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
# ==========================================

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(ambientzero_sensor C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

pico_sdk_init()

# Drivers
file(GLOB_RECURSE DRIVER_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/drivers/*.cpp"
)

# Services
file(GLOB_RECURSE SERVICE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*/*.c"
)

# Storage
file(GLOB_RECURSE STORAGE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/services/storage/*.cpp"
)

# Domain logic
file(GLOB_RECURSE DOMAIN_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/domain/*.cpp"
)

# Provisioning (POC)
file(GLOB_RECURSE PROVISIONING_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/domain/provisioning/*.cpp"
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

# -------------------------------------------------------
# Executable
# -------------------------------------------------------

add_executable(ambientzero_sensor
    src/app/main.cpp
    src/app/app_main.cpp

    ${DRIVER_SOURCES}
    ${SERVICE_SOURCES}
    ${STORAGE_SOURCES}
    ${DOMAIN_SOURCES}
    ${PROVISIONING_SOURCES}
    ${PROTOCOL_SOURCES}
    ${SYSTEM_SOURCES}
    ${CONFIG_SOURCES}
)


# -----------------------------------------
# RESERVE FLASH SPACE FOR SETTINGS
# -----------------------------------------
target_link_options(ambientzero_sensor PRIVATE
    -Wl,-T${CMAKE_CURRENT_LIST_DIR}/flash_config.ld
)

target_link_options(ambientzero_sensor PRIVATE
    -Wl,--section-start=.flash_config=0x101E0000
)

# -----------------------------------------
# MANUAL DEBUG LIGHTS FLAG (your toggle)
# -----------------------------------------
target_compile_definitions(ambientzero_sensor PRIVATE
    ENABLE_DEBUG_LIGHTS=0   # <--- set to 0 for production
    PIN_AHT_SDA=16
    PIN_AHT_SCL=17

    PIN_LED_R=2
    PIN_LED_G=1
    PIN_LED_B=0

    PIN_BATTERY_ADC=26
    PIN_OCTOCOUPLER=14
    PIN_SIGNAL_BUTTON=15

    HTTP_API_IP="10.0.0.1"
    HTTP_API_PORT=80

    MAX_WATCHDOG_CHUNK=8000

    AP_SSID="AmbientZero-Sensor"
    AP_PASSWORD=""
)

pico_set_program_name(ambientzero_sensor "ambientzero_sensor")
pico_set_program_version(ambientzero_sensor "0.1")

pico_enable_stdio_uart(ambientzero_sensor 0)
pico_enable_stdio_usb(ambientzero_sensor 0)

# -------------------------------------------------------
# Includes
# -------------------------------------------------------

target_include_directories(ambientzero_sensor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src/config
)


# -------------------------------------------------------
# Libraries
# -------------------------------------------------------

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

    # Optional but recommended if you want spi_deinit()
    hardware_spi

    # Wi-Fi stack
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_http
)


pico_add_extra_outputs(ambientzero_sensor)
