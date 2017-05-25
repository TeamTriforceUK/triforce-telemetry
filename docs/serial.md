# Introduction
This document describes the serial interfaces between the LPC1678 (mbed) and ESP8266 (WiFi) devices.
A custom humnan-readable protocol is defined below to communicate data between the decives.

A start byte and end byte are defined as follows:

| Purpose    | ASCII           | Dec | Hex  |
|------------|-----------------|-----|------|
| START_BYTE | BACKSPACE       | 127 | 0x7F |
| END_BYTE   | CARRIAGE RETURN | 13  | 0x0D |

This allows the serial interface to be used by humans as well as the microcontrollers (all charcters used are accessible using a standard keyboard).

## LPC1768 to ESP8266

| Variable   | Type   | Units | Example Command    |
|------------|--------|-------|--------------------|
| state      | string | N/A   | state ARMED        |
| ring_rpm   | float  | RPM   | ring_rpm 5000.98   |
| con_1_rpm  | float  | RPM   | con_1_rpm 50000.73 |
| con_2_rpm  | float  | RPM   | con_2_rpm 50000.47 |
| accel_x    | float  | m/s/s | accel_x 1.228      |
| accel_y    | float  | m/s/s | accel_y 1.228      |
| accel_z    | float  | m/s/s | accel_z 1.228      |
| pitch      | float  | deg   | pitch 23.29        |
| roll       | float  | deg   | roll  23.29        |
| yaw        | float  | deg   | yaw   23.29        |
| w_voltage  | float  | volts | w_voltage 23.29    |
| d_voltage  | float  | volts | d_voltage 23.29    |
| temp       | float  | C     | temp 23.29         |


## ESP8266 to LPC1768
