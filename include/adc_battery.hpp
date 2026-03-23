#ifndef ADC_BATTERY_HPP
#define ADC_BATTERY_HPP

#include <stdint.h>
#include "hardware/adc.h"

class BatteryADC {
public:
    BatteryADC(uint adc_input);

    void init();
    uint16_t read_raw();  // 0–4095

private:
    uint adc_input;
};

#endif
