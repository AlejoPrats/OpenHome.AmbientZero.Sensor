#pragma once
#include "drivers/ahtx.hpp"

/**
 * @brief Reads temperature and humidity from the AHT20 sensor.
 *
 * This function performs a single measurement using the provided
 * AHT20 driver instance and returns the results through the output
 * parameters.
 *
 * @param sensor        Reference to the AHT20 driver.
 * @param temperature   Output: measured temperature in °C.
 * @param humidity      Output: measured relative humidity in %RH.
 *
 * @return true if the measurement succeeded and valid data was
 *         retrieved. Returns false if the sensor failed to respond
 *         or produced invalid readings.
 *
 * @note This function contains no retries or delays. Any timing or
 *       error‑handling policy must be implemented by the caller.
 */
bool read_environment(AHT20 &sensor, float &temperature, float &humidity);
