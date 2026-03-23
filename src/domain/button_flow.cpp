#include "button_flow.hpp"
#include "pico/stdlib.h"

bool check_signal_button(RGBLed& led) {
    if (gpio_get(PIN_SIGNAL_BUTTON) == 0) {
        led.set_mode(LED_SIGNALING);
        return true;
    }
    return false;
}
