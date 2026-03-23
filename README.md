
# AmbientZero Sensor

AmbientZero Sensor is a lightweight, low‑power environmental monitoring device built on the Raspberry Pi Pico W.  
It measures temperature, humidity, and battery status, and periodically reports data to a remote API using an almost‑stateless design.

This project is written in modern C++ and follows a modular architecture designed for clarity, maintainability, and minimal resource usage on the RP2040.

---

## ✨ Features

- � Wi‑Fi connectivity with automatic reconnection flow  
- �️ AHTx temperature & humidity sensor driver  
- � Battery monitoring using an optocoupler‑isolated ADC path  
- � HTTP client with configurable endpoints  
- � Sleep & power‑management system for low‑power operation  
- � Stateless request/response flow  
- � Modular architecture (drivers, services, domain flows, system layer)  
- �️ Custom lwIP configuration for Pico W stability  

---

## � Project Structure

    include/ 		# Public headers 
    src/ 
    app/			# Application entry point and high-level logic 
    config/ 		# Device configuration and lwIP tuning 
    domain/ 		# State machines and operational flows 
    drivers/ 		# Hardware drivers (ADC, AHTx, RGB LED, etc.) 
    protocol/		# JSON parsing and server response handling 
    services/		# WiFi, HTTP client, scratch storage 
    system/			# Power management and system-level utilities 
    CMakeLists.txt	# Build configuration


This layout keeps hardware, logic, and protocol concerns cleanly separated.

---

## � Getting Started

### Requirements
- Raspberry Pi Pico W  
- CMake 3.13+  
- ARM GCC toolchain  
- Pico SDK 2.2.0  

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```
The resulting `.uf2` file will be located in the `build/` directory.

Flash it by holding BOOTSEL on the Pico W and copying the file to the mounted drive.

## ⚙️ Configuration

Device configuration is handled through: cmakelists.txt in this section

    target_compile_definitions(ambientzero_sensor PRIVATE
        ENABLE_DEBUG_LIGHTS=0   # <--- set to 0 for production
        PIN_AHT_SDA=16
        PIN_AHT_SCL=17
    
        PIN_LED_R=2
        PIN_LED_G=1
        PIN_LED_B=0
    
        PIN_BATTERY_ADC=26
        PIN_OCTOCOUPLER=14
        PIN_SIGNAL_BUTTON=15
    
        HTTP_API_IP="10.0.0.1"
        HTTP_API_PORT=80
    
        MAX_WATCHDOG_CHUNK=8000
    )

## � Testing

This project is tested manually on the physical device. There are no automated tests at this stage.

## � License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for full details.

:warning: :warning: :warning: **Do not connect the RPP to the USB while the battery is connected**, as far as I could investigate on the internet is that it will try to recharge the battery, but it might have some unintended results, and since I didn't want to blow any batteries or RPPs I didn't dig further into this issue, if that's the case in the future we might have rechargeable sensors, but for now lets not take risks  :sweat_smile:
