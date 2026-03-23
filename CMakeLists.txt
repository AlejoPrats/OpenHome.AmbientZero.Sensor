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

# -------------------------------------------------------
# Executable
# -------------------------------------------------------

add_executable(ambientzero_sensor
    # Entry point
    src/app/main.cpp
    src/app/app_main.cpp

    # Drivers
    src/drivers/ahtx.cpp
    src/drivers/rgb_led.cpp
    src/drivers/adc_battery.cpp
    src/drivers/sleeper.cpp

    # Services
    src/services/http_client.cpp
    src/services/wifi.cpp
    src/services/scratch_handler.cpp

    # Protocols
    src/protocol/json_parser.cpp
    src/protocol/server_response.cpp

    # System
    src/system/power_manager.cpp

    # Domain
    src/domain/hardware_init.cpp
    src/domain/button_flow.cpp
    src/domain/sensor_flow.cpp
    src/domain/wifi_flow.cpp
    src/domain/http_flow.cpp

    # Config
    src/config/device_config.cpp
    src/config/lwipopts.h
    src/config/lwipopts_custom.h
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
)

pico_set_program_name(ambientzero_sensor "ambientzero_sensor")
pico_set_program_version(ambientzero_sensor "0.1")

pico_enable_stdio_uart(ambientzero_sensor 0)
pico_enable_stdio_usb(ambientzero_sensor 1)

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
