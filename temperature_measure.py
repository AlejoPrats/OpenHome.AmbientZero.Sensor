import utime
from machine import Pin, I2C

import ahtx0

def get_temperature():
    # I2C for the Wemos D1 Mini with ESP8266
    i2c = I2C(0,scl=Pin(17), sda=Pin(16))

    # Create the sensor object using I2C
    sensor = ahtx0.AHT20(i2c)

    return sensor.temperature
