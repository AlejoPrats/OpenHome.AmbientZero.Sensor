#include "sensor_flow.hpp"
#include "pico/stdlib.h"

bool read_environment(AHT20& sensor, float& temperature, float& humidity) {
    for (int i = 0; i < 3; i++) {
        if (sensor.update()) {
            temperature = sensor.temperature();
            humidity = sensor.humidity();
            return true;
        }
        sleep_ms(50);
    }
    return false;
}
