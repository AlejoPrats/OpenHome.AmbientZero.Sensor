#ifndef AHTX_HPP
#define AHTX_HPP

#include <stdint.h>
#include "hardware/i2c.h"

/**
 * @brief Driver for the AHT10 temperature/humidity sensor.
 *
 * This class provides the minimal functionality required to operate
 * an AHT10 device over I2C. It handles initialization, triggering a
 * measurement, and exposing the latest temperature and humidity
 * values.
 *
 * The driver stores the raw 6‑byte measurement frame internally and
 * parses it into floating‑point values on update().
 *
 * @note No dynamic allocation is used. All buffers are fixed‑size.
 */
class AHT10
{
public:
    static const uint8_t ADDRESS = 0x38;

    /**
     * @brief Constructs an AHT10 bound to a specific I2C instance.
     *
     * @param i2c_port  Pointer to the RP2040 I2C peripheral.
     */
    AHT10(i2c_inst_t *i2c_port);

    /**
     * @brief Initializes the sensor.
     *
     * Sends the device‑specific initialization command and prepares
     * the sensor for measurement.
     *
     * @return true if initialization succeeded.
     */
    virtual bool init();

    /**
     * @brief Triggers a measurement and updates internal readings.
     *
     * Reads the 6‑byte measurement frame and updates the cached
     * temperature and humidity values.
     *
     * @return true if a valid measurement was received.
     */
    bool update();

    /**
     * @brief Returns the last measured temperature in °C.
     */
    float temperature();

    /**
     * @brief Returns the last measured relative humidity in %RH.
     */
    float humidity();

protected:
    i2c_inst_t *i2c;

    uint8_t buffer[6];

    float temp;
    float hum;

    /**
     * @brief Returns the initialization command for the sensor.
     *
     * AHT10 and AHT20 use different init sequences, so this method
     * is virtual and overridden by AHT20.
     */
    virtual uint8_t init_command();

    /**
     * @brief Issues a soft reset to the sensor.
     */
    void reset();
};

/**
 * @brief AHT20 variant of the AHT10 driver.
 *
 * The AHT20 uses the same measurement protocol as the AHT10 but
 * requires a different initialization command. All other behavior
 * is inherited from AHT10.
 */
class AHT20 : public AHT10
{
public:
    /**
     * @brief Constructs an AHT20 bound to a specific I2C instance.
     */
    AHT20(i2c_inst_t *i2c_port);

protected:
    /**
     * @brief Returns the AHT20‑specific initialization command.
     */
    uint8_t init_command() override;
};

#endif
