#include "domain/hardware_init.hpp"
#include "drivers/rgb_led.hpp"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

void init_hardware() {
    i2c_init(i2c0, 100000);

    gpio_set_function(PIN_AHT_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_AHT_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_AHT_SDA);
    gpio_pull_up(PIN_AHT_SCL);

    gpio_init(PIN_SIGNAL_BUTTON);
    gpio_set_dir(PIN_SIGNAL_BUTTON, GPIO_IN);
    gpio_pull_up(PIN_SIGNAL_BUTTON);

    led.init();
}
