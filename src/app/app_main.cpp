#include "app/app_main.hpp"

#include "domain/hardware_init.hpp"
#include "domain/sensor_flow.hpp"
#include "domain/wifi_flow.hpp"
#include "domain/http_flow.hpp"

#include "drivers/rgb_led.hpp"
#include "drivers/adc_battery.hpp"
#include "config/device_config.hpp"

#include "portal/provisioning_flow.hpp"

#include "pico/stdlib.h"
#include <cstdio>

static AHT20 sensor(i2c0);
static BatteryADC battery(PIN_BATTERY_ADC);

void app_main_start_provisioning()
{
    init_hardware();
    led.init();
    provisioning_setup();
}

extern const uint8_t __flash_config_start__;
extern const uint8_t __flash_config_end__;

void app_main_run_cycle(bool isSignaling)
{
    sensor.init();
    battery.init();

    DeviceConfig cfg{};
    bool has_config = load_config(cfg);

    if (!has_config)
    {
        printf("config not found \n");
    }

    if (isSignaling)
    {
        led.set_mode_blocking(LED_SIGNALING);
    }

    if (!WifiFlow::wifi_connect_flow(cfg))
    {
        led.set_mode_blocking(LED_ERROR);
        return;
    }

    float temperature = 0;
    float humidity = 0;

    if (!read_environment(sensor, temperature, humidity))
    {
        led.set_mode_blocking(LED_ERROR);
        return;
    }

    uint16_t battery_raw = battery.read_raw();

    send_measurement_flow(
        temperature,
        humidity,
        battery_raw,
        isSignaling,
        led,
        cfg);
}
