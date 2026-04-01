#include "domain/button_flow.hpp"
#include "drivers/rgb_led.hpp"
#include "pico/stdlib.h"

static ButtonStatus state = ButtonStatus::IDLE;
static uint32_t pressStart = 0;
static bool oneBlinkDone = false;
static bool twoBlinkDone = false;

static inline bool button_pressed() {
    return gpio_get(PIN_SIGNAL_BUTTON) == 0;
}

ButtonStatus button_status() {
    return gpio_get(PIN_SIGNAL_BUTTON) == 0
        ? ButtonStatus::PRESSED
        : ButtonStatus::IDLE;
}

ButtonAction button_poll() {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (state) {

    case ButtonStatus::IDLE:
        if (button_pressed()) {
            state = ButtonStatus::PRESSED;
            pressStart = now;
            oneBlinkDone = false;
            twoBlinkDone = false;
        }
        break;

    case ButtonStatus::PRESSED: {
        uint32_t duration = now - pressStart;

        // Blink once at 100ms
        if (duration >= 100 && !oneBlinkDone) {
            led.set_mode(LedMode::LED_BLINK_ONCE);
            oneBlinkDone = true;
        }

        // Blink twice at 5 seconds
        if (duration >= 5000 && !twoBlinkDone) {
            led.set_mode(LedMode::LED_BLINK_TWICE);
            twoBlinkDone = true;
        }

        // Button released → classify
        if (!button_pressed()) {
            state = ButtonStatus::RELEASED;
        }
        break;
    }

    case ButtonStatus::RELEASED: {
        uint32_t duration = now - pressStart;
        state = ButtonStatus::IDLE;

        if (duration >= 100 && duration <= 5000)
            return ButtonAction::SIGNAL_PRESS;

        if (duration > 5000 && duration <= 15000)
            return ButtonAction::PROVISION_PRESS;

        return ButtonAction::NORMAL_BOOT;
    }
    }

    return ButtonAction::NONE;
}
