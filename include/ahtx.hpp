#ifndef AHTX_HPP
#define AHTX_HPP

#include <stdint.h>
#include "hardware/i2c.h"

class AHT10 {
public:

    static const uint8_t ADDRESS = 0x38;

    AHT10(i2c_inst_t *i2c_port);

    virtual bool init();
    bool update();

    float temperature();
    float humidity();

protected:

    i2c_inst_t *i2c;

    uint8_t buffer[6];

    float temp;
    float hum;

    virtual uint8_t init_command();

    void reset();
};


class AHT20 : public AHT10 {
public:

    AHT20(i2c_inst_t *i2c_port);

protected:

    uint8_t init_command() override;
};

#endif