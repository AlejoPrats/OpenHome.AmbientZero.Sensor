from picozero import RGBLED

rgb = RGBLED(red = 2, green = 1, blue = 0)
red = (255, 0, 0)
green = (0, 255, 0)
blue = (0, 0, 255)
purple = (255, 0, 255)
off = (0, 0, 0)


def blink_led(message):    
    if message == "OK":
        rgb.blink((1, 0.5, 1, 0.5), colors=(blue, off, green, off), wait=True, n=2)
    elif message == "ERROR":
        rgb.blink((1, 0.5, 1, 0.5), colors=(blue, off, red, off), wait=True, n=2)
    elif message == "SIGNALING":
        rgb.pulse((2, 1, 2, 1), colors=(purple, off, purple, off), wait=True, n=1)