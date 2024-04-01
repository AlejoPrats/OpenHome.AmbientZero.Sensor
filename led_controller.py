from machine import Pin, Timer
led = Pin("LED", Pin.OUT)

def toggle_led():
    led.toggle()