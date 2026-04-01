#ifndef ADC_BATTERY_HPP
#define ADC_BATTERY_HPP

#include <stdint.h>
#include "hardware/adc.h"

/**
 * @brief Simple wrapper for reading the battery ADC channel.
 *
 * This class encapsulates the minimal logic required to sample the
 * battery voltage through the RP2040 ADC. It performs no scaling or
 * interpretation; callers are responsible for converting the raw
 * 0–4095 value into a voltage or percentage.
 *
 * @note The ADC input pin must be configured for analog mode before
 *       calling read_raw(). Initialization is handled by init().
 */
class BatteryADC
{
public:
    /**
     * @brief Constructs a BatteryADC bound to a specific ADC input.
     *
     * @param adc_input  ADC channel index (0–3 for GPIO26–29).
     */
    BatteryADC(uint adc_input);

    /**
     * @brief Initializes the ADC hardware for battery measurement.
     *
     * Enables the ADC subsystem and selects the configured input
     * channel. Must be called once before read_raw().
     */
    void init();

    /**
     * @brief Reads a raw ADC sample from the configured channel.
     *
     * @return 12‑bit ADC value in the range 0–4095.
     *
     * @note This function performs a single blocking ADC conversion.
     *       Any filtering or averaging must be implemented by the caller.
     */
    uint16_t read_raw(); // 0–4095

private:
    uint adc_input;
};

#endif
