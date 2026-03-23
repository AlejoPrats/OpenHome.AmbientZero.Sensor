#include "adc_battery.hpp"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "stdio.h"

BatteryADC::BatteryADC(uint adc_input)
    : adc_input(adc_input) {}

void BatteryADC::init()
{
    adc_init();
    adc_gpio_init(PIN_BATTERY_ADC);
}

uint16_t BatteryADC::read_raw()
{
    gpio_init(PIN_OCTOCOUPLER);
    gpio_set_dir(PIN_OCTOCOUPLER, GPIO_OUT);
    gpio_put(PIN_OCTOCOUPLER, 1);

    sleep_ms(5); // let the optocoupler settle

    // 2. Read ADC0 (GPIO26)
    adc_select_input(0);

    //Since the RP2400 adc read returns 12-bit value we transform it to 16-bit 
    uint16_t raw = adc_read() << 4;

    // 3. Turn off optocoupler
    gpio_put(PIN_OCTOCOUPLER, 0);

    return raw;
}
