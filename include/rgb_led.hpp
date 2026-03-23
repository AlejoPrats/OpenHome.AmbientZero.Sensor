#pragma once
#include <cstdint>

enum LedMode {
    LED_OK,
    LED_ERROR,
    LED_CAPTIVE,
    LED_SIGNALING,
    LED_OFF
};

class RGBLed {
public:
    RGBLed(uint8_t r, uint8_t g, uint8_t b);

    void init();
    void set_mode(LedMode m);

private:
    void set_rgb(uint8_t r, uint8_t g, uint8_t b);

    uint8_t pin_r, pin_g, pin_b;
};
