/**
* @file config.h
* @author Cameron A. Craig
* @date 25 May 2017
* @copyright 2017 Cameron A. Craig
* @brief <brief>
*
*/

#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#define VERSION "0.1.0"

#define LED_PIN 2
#define READY_PIN 15

#define UART_START 0x7F  // (backspace)
#define UART_END 0xD  // (carriage return

#define DEFAULT_WIFI_SSID "SKY2E317"
#define DEFAULT_WIFI_PASS "CDBQXBAX"

#define MAX_JSON_STRLEN 100U

#endif  // INCLUDE_CONFIG_H_
