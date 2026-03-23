#pragma once
#include "rgb_led.hpp"
#include "device_config.hpp"

bool send_measurement_flow(
    float temperature,
    float humidity,
    uint16_t battery_raw,
    bool isSignaling,
    RGBLed& led,
    const DeviceConfig& cfg
);
