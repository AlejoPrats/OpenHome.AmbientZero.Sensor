import time
import wifiConnect
import led_controller
import device_id
import log_data
import temperature_measure
import rgb_led
import urequests as requests
from machine import deepsleep
from picozero import Button

signal_button = Button(15)
is_signaling = False

try:
    led_controller.toggle_led()
    if(signal_button.is_active):
        is_signaling = True
        rgb_led.blink_led("SIGNALING")
     
    wifiConnect.connect_to_internet('AmbientZero','')
    log_data.log_data(device_id.get_id(), temperature_measure.get_temperature(), is_signaling)
    rgb_led.blink_led("OK")
except:
    rgb_led.blink_led("ERROR")
    print("Error")
finally:
    led_controller.toggle_led()
    wifiConnect.disconnect()
    deepsleep(1200000)


    