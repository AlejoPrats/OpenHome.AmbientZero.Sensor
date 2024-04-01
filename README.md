# OpenHome.AmbientZero.Sensor

## Ambient Zero Sensor

This project contains the code that goes inside the Raspberry Pi Pico, it's coded in MicroPython, a minimal implementation of Python 3
connect to the AmbientZero Access point
- Is designed to run in a loop that will read the information from the temperature sensor (AHT20)
- Do a PUT request to the [AmbientZero.API](https://github.com/AlejoPrats/OpenHome.AmbientZero.Api "AmbientZero.API")
- In case of success it will blink the RGB LED blue and green
- In case of failure it will blink the RBG LED blue and red :warning: (Sometimes it randomly fails to connect to the AP, I am investigating this since it can be the Linux AP configuration or something with the RPP)
- After that, it will get into a deep sleep for 20 minutes and repeat the loop

It can signal the AP to identify the sensor in the App and rename it accordingly

### What is missing?

- The API after the put will return if the lights should blink or not, this is handled by modifying the settings of the sensor from the APP
- The battery measurement, right now hardcoded the value in the code, I need to test the whole cycle to see the values that I get from the ADC and when it stops working
- How long does the battery last, the sensors have been running here for more than a month, but I am not sure exactly how long the battery lasts

## How to Install in the RPP

1) Download the [Thonny Ide](https://apexcharts.github.io/Blazor-ApexCharts/ "Thonny Ide") (don't worry it is multiplatform :grinning:)
2) Install the Ide and connect the RPP to any USB port in your computer holding the BOOTSEL button
3) Thonny will give you the option to install MicroPython when you click on the lower-right corner of the Ide
4) Select the Target Volume that should show something like "RPI-RP2"
5) Select the MicroPython family and choose "RP2"
6) Lastly the variant should be "Raspberry Pi • Pico W / Pico WH"
7) The version will be filled automatically with the latest the time of this writing was 1.22.2, click on the install button and wait until it finishes
8) Once it finishes the MicroPython installation it will reboot automatically into the development mode
9) click again on the lower-right corner and select the option that reads MicroPython (Raspberry Pi Pico)
10) You should see that the left side of the IDE is split into 2, the top side will say This computer "{path}" and the lower part will say "Raspberry Pi Pico" 
11) Navigate where you have the Python files and upload them to the RPP
12) :warning: **This step is very important** :warning: Double click the id.txt file that is on the Raspberry Pi Pico pane, and write a GUID (you can use any online GUID/UUID generator) copy and paste the value and save the file (this is the unique identifier of the sensor)
13) Now you can connect the RPP to the board and it should start working as was explained in the project description

:warning: :warning: :warning: **Do not connect the RPP to the USB while the battery is connected**, as far as I could investigate on the internet is that it will try to recharge the battery, but it might have some unintended results, and since I didn't want to blow any batteries or RPPs I didn't dig further into this issue, if that's the case in the future we might have rechargeable sensors, but for now lets not take risks  :sweat_smile:
