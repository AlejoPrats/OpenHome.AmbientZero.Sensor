#import time
#import ntptime
import urequests as requests
from machine import deepsleep
from Measurement import Measurement
import json
        
def log_data(device_id, temperature, is_signaling):
    
    test = requests.get('http://10.0.0.1/swagger/index.html')
    print(test.status_code)
    
    url = 'http://10.0.0.1/AmbientTemperature'
    payload = Measurement(temperature, device_id, 100, is_signaling)
    request = requests.put(url, headers={"Content-Type": "application/json", "Accept": "application/json"}, data=json.dumps(payload.__dict__))
    request.close()
        
    if request.status_code != 202:
        print('Request failed with code: {}'.format(request.status_code))
    else:
        print('Posted information succesfully')

    
#    ntptime.host = "1.europe.pool.ntp.org"
#
#    file_object = open('/logs.txt', 'a')
#    ntptime.settime()
#    actual_time = time.localtime()
#    
#    file_object.write("{}/{}/{} {}:{}:{} \n".format(actual_time[1],actual_time[2],actual_time[0],actual_time[3],actual_time[4],actual_time[5]))
#    file_object.close()
#    with open('logs.txt','r') as file:
#        content = file.read()
#        print(content)

