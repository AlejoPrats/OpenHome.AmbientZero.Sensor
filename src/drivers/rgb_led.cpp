#include "rgb_led.hpp"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

RGBLed::RGBLed(uint8_t r, uint8_t g, uint8_t b)
    : pin_r(r), pin_g(g), pin_b(b)
{}

void RGBLed::init() {
    gpio_set_function(pin_r, GPIO_FUNC_PWM);
    gpio_set_function(pin_g, GPIO_FUNC_PWM);
    gpio_set_function(pin_b, GPIO_FUNC_PWM);

    uint slice_r = pwm_gpio_to_slice_num(pin_r);
    uint slice_g = pwm_gpio_to_slice_num(pin_g);
    uint slice_b = pwm_gpio_to_slice_num(pin_b);

    pwm_set_wrap(slice_r, 255);
    pwm_set_wrap(slice_g, 255);
    pwm_set_wrap(slice_b, 255);

    pwm_set_enabled(slice_r, true);
    pwm_set_enabled(slice_g, true);
    pwm_set_enabled(slice_b, true);
}

void RGBLed::set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    pwm_set_gpio_level(pin_r, r);
    pwm_set_gpio_level(pin_g, g);
    pwm_set_gpio_level(pin_b, b);
}

void RGBLed::set_mode(LedMode m) {

    switch (m) {

        // ---------------------------------------------------------
        // LED_OK → blue → green cycle, 2 times
        // ---------------------------------------------------------
        case LED_OK:
            for (int i = 0; i < 2; i++) {
                set_rgb(0, 0, 255);   // blue
                sleep_ms(300);
                set_rgb(0, 0, 0);
                sleep_ms(150);

                set_rgb(0, 255, 0);   // green
                sleep_ms(300);
                set_rgb(0, 0, 0);
                sleep_ms(150);
            }
            break;

        // ---------------------------------------------------------
        // LED_ERROR → blue → red cycle, 2 times
        // ---------------------------------------------------------
        case LED_ERROR:
            for (int i = 0; i < 2; i++) {
                set_rgb(0, 0, 255);   // blue
                sleep_ms(300);
                set_rgb(0, 0, 0);
                sleep_ms(150);

                set_rgb(255, 0, 0);   // red
                sleep_ms(300);
                set_rgb(0, 0, 0);
                sleep_ms(150);
            }
            break;

        // ---------------------------------------------------------
        // LED_CAPTIVE → blue breathing for 10 minutes
        // ---------------------------------------------------------
        case LED_CAPTIVE: {
            const int total_ms = 10 * 60 * 1000; // 10 minutes
            const int step_ms = 20;              // smooth breathing
            const int steps = total_ms / step_ms;

            for (int i = 0; i < steps; i++) {
                float t = (i % 200) / 200.0f; // breathing cycle
                float brightness = (t < 0.5f) ? (t * 2) : (2 - t * 2);
                set_rgb(0, 0, (uint8_t)(255 * brightness));
                sleep_ms(step_ms);
            }
            break;
        }

        // ---------------------------------------------------------
        // LED_SIGNALING → purple breathing 2 times
        // ---------------------------------------------------------
        case LED_SIGNALING: {
            const int cycles = 2;
            const int cycle_ms = 2000;
            const int step_ms = 20;
            const int steps = cycle_ms / step_ms;

            for (int c = 0; c < cycles; c++) {
                for (int i = 0; i < steps; i++) {
                    float t = i / (float)steps;
                    float brightness = (t < 0.5f) ? (t * 2) : (2 - t * 2);
                    set_rgb((uint8_t)(255 * brightness), 0, (uint8_t)(255 * brightness));
                    sleep_ms(step_ms);
                }
            }
            break;
        }

        // ---------------------------------------------------------
        // LED_OFF → off
        // ---------------------------------------------------------
        case LED_OFF:
        default:
            set_rgb(0, 0, 0);
            break;
    }
}
