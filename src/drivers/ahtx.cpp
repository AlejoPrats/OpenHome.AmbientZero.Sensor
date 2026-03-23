#include "ahtx.hpp"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define CMD_TRIGGER 0xAC
#define CMD_SOFTRESET 0xBA

#define STATUS_BUSY 0x80
#define STATUS_CALIBRATED 0x08

// Tunables
#define AHTX_INIT_DELAY_MS 40
#define AHTX_RESET_DELAY_MS 20
#define AHTX_MEAS_DELAY_MS 40
#define AHTX_BUSY_TIMEOUT_MS 200 // total wait for BUSY to clear
#define AHTX_I2C_TIMEOUT_US 5000 // per I2C op

AHT10::AHT10(i2c_inst_t *i2c_port)
    : i2c(i2c_port), temp(0.0f), hum(0.0f) {}

void AHT10::reset()
{
    uint8_t cmd = CMD_SOFTRESET;

    int res = i2c_write_timeout_us(
        i2c, ADDRESS, &cmd, 1, false, AHTX_I2C_TIMEOUT_US);
    if (res < 0)
    {
        // Optional: log or set an internal error flag
        return;
    }

    sleep_ms(AHTX_RESET_DELAY_MS);
}

uint8_t AHT10::init_command()
{
    return 0xE1; // AHT10
}

bool AHT10::init()
{
    sleep_ms(20);

    reset();

    uint8_t cmd[3] = {init_command(), 0x08, 0x00};

    int res = i2c_write_timeout_us(
        i2c, ADDRESS, cmd, 3, false, AHTX_I2C_TIMEOUT_US);
    if (res < 0)
    {
        return false;
    }

    sleep_ms(AHTX_INIT_DELAY_MS);

    uint8_t status = 0;
    res = i2c_read_timeout_us(
        i2c, ADDRESS, &status, 1, false, AHTX_I2C_TIMEOUT_US);
    if (res < 0)
    {
        return false;
    }

    // � ADD THIS — MicroPython discards the first reading
    update();
    sleep_ms(40);

    return (status & STATUS_CALIBRATED) != 0;
}

bool AHT10::update()
{
    uint8_t cmd[3] = {CMD_TRIGGER, 0x33, 0x00};

    int res = i2c_write_timeout_us(
        i2c, ADDRESS, cmd, 3, false, AHTX_I2C_TIMEOUT_US);
    if (res < 0)
    {
        return false;
    }

    sleep_ms(40);

    // Poll with timeout
    absolute_time_t deadline = make_timeout_time_ms(AHTX_BUSY_TIMEOUT_MS);
    uint8_t local_buf[6];

    do
    {
        res = i2c_read_timeout_us(
            i2c, ADDRESS, local_buf, 6, false, AHTX_I2C_TIMEOUT_US);
        if (res < 0)
        {
            return false;
        }

        if (!(local_buf[0] & STATUS_BUSY))
        {
            break;
        }

        sleep_ms(AHTX_MEAS_DELAY_MS);
    } while (!time_reached(deadline));

    if (local_buf[0] & STATUS_BUSY)
    {
        // Timed out waiting for measurement
        return false;
    }

    // Parse measurement
    uint32_t raw_hum =
        ((uint32_t)local_buf[1] << 12) |
        ((uint32_t)local_buf[2] << 4) |
        (local_buf[3] >> 4);

    hum = (raw_hum * 100.0f) / 1048576.0f;

    uint32_t raw_temp =
        ((uint32_t)(local_buf[3] & 0x0F) << 16) |
        ((uint32_t)local_buf[4] << 8) |
        local_buf[5];

    temp = ((raw_temp * 200.0f) / 1048576.0f) - 50.0f;

    return true;
}

float AHT10::temperature()
{
    return temp;
}

float AHT10::humidity()
{
    return hum;
}

/* -------- AHT20 -------- */

AHT20::AHT20(i2c_inst_t *i2c_port)
    : AHT10(i2c_port) {}

uint8_t AHT20::init_command()
{
    return 0xBE; // AHT20 calibration command
}
