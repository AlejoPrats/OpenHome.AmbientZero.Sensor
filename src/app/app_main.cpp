#include "app_main.hpp"

#include "hardware_init.hpp"
#include "button_flow.hpp"
#include "sensor_flow.hpp"
#include "wifi_flow.hpp"
#include "http_flow.hpp"

#include "rgb_led.hpp"
#include "adc_battery.hpp"
#include "device_config.hpp"

#include "pico/stdlib.h"

static char deviceId[64] = {0};

void app_main() {
    // -----------------------------
    // HARDWARE INIT
    // -----------------------------
    init_hardware();

    RGBLed led(PIN_LED_R, PIN_LED_G, PIN_LED_B);
    AHT20 sensor(i2c0);
    BatteryADC battery(PIN_BATTERY_ADC);

    led.init();
    sensor.init();
    battery.init();

    // -----------------------------
    // LOAD CONFIG
    // -----------------------------
    DeviceConfig cfg{};
    load_config(cfg);

    // -----------------------------
    // BUTTON CHECK
    // -----------------------------
    bool isSignaling = check_signal_button(led);

    // -----------------------------
    // WIFI CONNECT
    // -----------------------------
    if (!wifi_connect_flow(cfg)) {
        led.set_mode(LED_ERROR);
        return; // no cleanup needed
    }

    // -----------------------------
    // SENSOR READ
    // -----------------------------
    float temperature = 0;
    float humidity = 0;

    if (!read_environment(sensor, temperature, humidity)) {
        led.set_mode(LED_ERROR);
        return;
    }

    uint16_t battery_raw = battery.read_raw();

    // -----------------------------
    // HTTP SEND
    // -----------------------------
    if (!send_measurement_flow(
            temperature,
            humidity,
            battery_raw,
            isSignaling,
            led,
            cfg
        )) 
    {
        return;
    }
}
