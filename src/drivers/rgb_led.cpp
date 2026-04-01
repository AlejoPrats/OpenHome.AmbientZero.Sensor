#include "drivers/rgb_led.hpp"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "pico/time.h"

RGBLed led(PIN_LED_R, PIN_LED_G, PIN_LED_B);

RGBLed::RGBLed(uint8_t r, uint8_t g, uint8_t b)
    : pin_r(r), pin_g(g), pin_b(b)
{
}

void RGBLed::init()
{
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

void RGBLed::set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    pwm_set_gpio_level(pin_r, r);
    pwm_set_gpio_level(pin_g, g);
    pwm_set_gpio_level(pin_b, b);
}

//
// -------------------------------------------------------------
//  BLOCKING VERSION (your original behavior)
// -------------------------------------------------------------
//
void RGBLed::set_mode_blocking(LedMode m)
{

    switch (m)
    {

    // ---------------------------------------------------------
    // LED_OK → blue → green cycle, 2 times
    // ---------------------------------------------------------
    case LED_OK:
        for (int i = 0; i < 2; i++)
        {
            set_rgb(0, 0, 255); // blue
            sleep_ms(300);
            set_rgb(0, 0, 0);
            sleep_ms(150);

            set_rgb(0, 255, 0); // green
            sleep_ms(300);
            set_rgb(0, 0, 0);
            sleep_ms(150);
        }
        break;

    // ---------------------------------------------------------
    // LED_ERROR → blue → red cycle, 2 times
    // ---------------------------------------------------------
    case LED_ERROR:
        for (int i = 0; i < 2; i++)
        {
            set_rgb(0, 0, 255); // blue
            sleep_ms(300);
            set_rgb(0, 0, 0);
            sleep_ms(150);

            set_rgb(255, 0, 0); // red
            sleep_ms(300);
            set_rgb(0, 0, 0);
            sleep_ms(150);
        }
        break;

    // ---------------------------------------------------------
    // LED_SIGNALING (blocking version)
    // ---------------------------------------------------------
    case LED_SIGNALING:
    {
        const int cycles = 2;
        const int cycle_ms = 2000;
        const int step_ms = 20;
        const int steps = cycle_ms / step_ms;

        for (int c = 0; c < cycles; c++)
        {
            for (int i = 0; i < steps; i++)
            {
                float t = i / (float)steps;
                float brightness = (t < 0.5f) ? (t * 2) : (2 - t * 2);
                uint8_t v = (uint8_t)(255 * brightness);
                set_rgb(v, 0, v);
                sleep_ms(step_ms);
            }
        }
        break;
    }

    // ---------------------------------------------------------
    // LED_CAPTIVE (blocking version)
    // ---------------------------------------------------------
    case LED_CAPTIVE:
    {
        const int total_ms = 10 * 60 * 1000; // 10 minutes
        const int step_ms = 20;
        const int steps = total_ms / step_ms;

        for (int i = 0; i < steps; i++)
        {
            float t = (i % 200) / 200.0f;
            float brightness = (t < 0.5f) ? (t * 2) : (2 - t * 2);
            set_rgb(0, 0, (uint8_t)(255 * brightness));
            sleep_ms(step_ms);
        }
        break;
    }

    // ---------------------------------------------------------
    // LED_OFF
    // ---------------------------------------------------------
    case LED_OFF:
    default:
        set_rgb(0, 0, 0);
        break;
    }
}

//
// -------------------------------------------------------------
//  NON-BLOCKING VERSION
// -------------------------------------------------------------
//
void RGBLed::set_mode(LedMode m)
{
    current_mode = m;
    step = 0;
    last_update_ms = to_ms_since_boot(get_absolute_time());

    // Immediate effect for OFF
    if (m == LED_OFF)
    {
        set_rgb(0, 0, 0);
    }
}

//
// -------------------------------------------------------------
//  poll() — drives non-blocking patterns
// -------------------------------------------------------------
//
void RGBLed::poll()
{
    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (current_mode)
    {

    // ---------------------------------------------------------
    // LED_CAPTIVE → blue breathing (non-blocking)
    // ---------------------------------------------------------
    case LED_CAPTIVE:
    {
        const int step_ms = 20;
        if (now - last_update_ms < step_ms)
            return;
        last_update_ms = now;

        float t = (step % 200) / 200.0f;
        float brightness = (t < 0.5f) ? (t * 2) : (2 - t * 2);

        set_rgb(0, 0, (uint8_t)(255 * brightness));
        step++;
        break;
    }

    // ---------------------------------------------------------
    // LED_SIGNALING → purple breathing (non-blocking)
    // ---------------------------------------------------------
    case LED_SIGNALING:
    {
        const int step_ms = 20;
        if (now - last_update_ms < step_ms)
            return;
        last_update_ms = now;

        float t = (step % 200) / 200.0f;
        float brightness = (t < 0.5f) ? (t * 2) : (2 - t * 2);

        uint8_t v = (uint8_t)(255 * brightness);
        set_rgb(v, 0, v);
        step++;
        break;
    }

    case LED_BLINK_ONCE:
    {
        const int step_ms = 20;
        if (now - last_update_ms < step_ms)
            return;
        last_update_ms = now;

        if (step == 0)
        {
            set_rgb(255, 255, 255);
        }
        else if (step == 10)
        {
            set_rgb(0, 0, 0);
            current_mode = LED_OFF;
        }
        step++;
        break;
    }

    case LED_BLINK_TWICE:
    {
        const int step_ms = 20;
        if (now - last_update_ms < step_ms)
            return;
        last_update_ms = now;

        if (step == 0 || step == 20)
        {
            set_rgb(255, 255, 255);
        }
        else if (step == 10 || step == 30)
        {
            set_rgb(0, 0, 0);
        }
        else if (step > 40)
        {
            current_mode = LED_OFF;
        }
        step++;
        break;
    }

    // ---------------------------------------------------------
    // LED_OK / LED_ERROR (non-blocking version = immediate OFF)
    // ---------------------------------------------------------
    case LED_OK:
    case LED_ERROR:
        // These are meant to be blocking patterns.
        // Non-blocking version just turns LED off.
        set_rgb(0, 0, 0);
        current_mode = LED_OFF;
        break;

    // ---------------------------------------------------------
    // LED_OFF
    // ---------------------------------------------------------
    case LED_OFF:
    default:
        // nothing to do
        break;
    }
}
