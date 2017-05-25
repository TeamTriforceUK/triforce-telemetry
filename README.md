# triforce-telemetry

## Introduction
This firmware runs on the ESP8266, part of the electronics used on Triforce. The ESP8266 allows us to communicate telemetry from Triforce to connected mobile devices (tablets, laptops, mobile phones). We also control non-safety critical settings such as LED lights from the mobile device.

## Hardware Configuration
- WiFi Router
- LPC1768
- ESP8266
- BNO055


## Flashing
```
make flash -j4 ESPPORT=/dev/ttyUSB0
```
