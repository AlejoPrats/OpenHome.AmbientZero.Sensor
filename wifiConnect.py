import time
import network

def connect_to_internet(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(ssid)
    
    max_wait = 30
    while max_wait > 0:
        if wlan.status() < 0 or wlan.status() >= 3:
            break
        max_wait -= 1
        print('Waiting for connection...')
        time.sleep(1)
        
    if wlan.status() != 3:
        print(wlan.status())
        raise RuntimeError('Network Connection Failed')
    else:
        print('Connected')
        status = wlan.ifconfig()
        print(wlan.isconnected())
        print(wlan.ifconfig())

def disable_wlan():
    wlan = network.WLAN(network.STA_IF)
    wlan.deinit()
    
def disconnect():
    wlan = network.WLAN(network.STA_IF)
    wlan.disconnect()
    wlan.deinit()