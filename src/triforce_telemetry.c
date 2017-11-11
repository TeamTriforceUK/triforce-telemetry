/**
* @file http_server.c
* @author https://github.com/lujji, Cameron A. Craig
* @date 25 May 2017
* @copyright Cameron A. Craig
* @brief Connects to an access point and hosts a HTTP server.
*        A web page is displayed, displaying telemetry data and
*        allowing configuration of non safety-critical settings.
*/

#include <espressif/esp_common.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>
#include <httpd/httpd.h>

#include "config.h"
#include "tele_param.h"
#include "thread_args.h"
#include "recv_commands.h"
#include "states.h"

/* Allows thread_args to be accessible to callback functions that do not easily
   support (void *) arguments. */
thread_args_t *gargs;

enum {
    SSI_UPTIME,
    SSI_FREE_HEAP,
    SSI_LED_STATE
};

int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen) {
    switch (iIndex) {
        case SSI_UPTIME:
            snprintf(pcInsert, iInsertLen, "%d",
                    xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
            break;
        case SSI_FREE_HEAP:
            snprintf(pcInsert, iInsertLen, "%d", (int) xPortGetFreeHeapSize());
            break;
        case SSI_LED_STATE:
            snprintf(pcInsert, iInsertLen, (GPIO.OUT & BIT(LED_PIN)) ? "Off" : "On");
            break;
        default:
            snprintf(pcInsert, iInsertLen, "N/A");
            break;
    }

    /* Tell the server how many characters to insert */
    return (strlen(pcInsert));
}

char *websocket_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    return "/index.html";
}

void websocket_task(void *pvParameter) {
    struct tcp_pcb *pcb = (struct tcp_pcb *) pvParameter;

    for (;;) {
        if (pcb == NULL || pcb->state != ESTABLISHED) {
            printf("Connection closed, deleting task\n");
            break;
        }

        int uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        int heap = (int) xPortGetFreeHeapSize();

        /* Generate response in JSON format */
        char response[512];
        memset(response, 0x00, 512);

        int i;
        int len = 0;
        for(i = 0; i < gargs->num_mbed_params; i++) {
          switch (gargs->mbed_params[i].type) {
            case CT_FLOAT:
              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "{\"name\": \"%s\", \"type\": \"%s\", \"unit\": \"%s\", \"value\": \"%.2f\"}\r",
                gargs->mbed_params[i].name,
                tele_command_type_to_string(gargs->mbed_params[i].type),
                tele_command_unit_to_string(gargs->mbed_params[i].unit),
                gargs->mbed_params[i].param.f
              );
            break;
            case CT_INT32:
              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "{\"name\": \"%s\", \"type\": \"%s\", \"unit\": \"%s\", \"value\": \"%d\"}\r",
                gargs->mbed_params[i].name,
                tele_command_type_to_string(gargs->mbed_params[i].type),
                tele_command_unit_to_string(gargs->mbed_params[i].unit),
                gargs->mbed_params[i].param.i32
              );
              break;
            case CT_BOOLEAN:
              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "{\"name\": \"%s\", \"type\": \"%s\", \"unit\": \"%s\", \"value\": \"%s\"}\r",
                gargs->mbed_params[i].name,
                tele_command_type_to_string(gargs->mbed_params[i].type),
                tele_command_unit_to_string(gargs->mbed_params[i].unit),
                gargs->mbed_params[i].param.b ? "ON" : "OFF"
              );
              break;
            case CT_NONE:
            default:
              printf("Type not yet supported for streaming.\r\n");
              break;
          }
        }

        if (len < sizeof (response)) {
            websocket_write(pcb, (unsigned char *) response, len, WS_TEXT_MODE);
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/**
 * This function is called when websocket frame is received.
 *
 * Note: this function is executed on TCP thread and should return as soon
 * as possible.
 */
void websocket_cb(struct tcp_pcb *pcb, uint8_t *data, u16_t data_len, uint8_t mode) {
    printf("[websocket_callback]:\n%.*s\n", (int) data_len, (char*) data);

    uint8_t response[2];
    uint16_t val;

    switch (data[0]) {
        case 'A': // ADC
            /* This should be done on a separate thread in 'real' applications */
            val = sdk_system_adc_read();
            break;
        case 'D': // Disable LED
            // gargs->esp_params.led = false;
            val = 0xDEAD;
            break;
        case 'E': // Enable LED
            // gargs->esp_params.led = true;
            val = 0xBEEF;
            break;
        default:
            printf("Unknown command\n");
            val = 0;
            break;
    }

    response[1] = (uint8_t) val;
    response[0] = val >> 8;

    websocket_write(pcb, response, 2, WS_BIN_MODE);
}

/**
 * This function is called when new websocket is open and
 * creates a new websocket_task if requested URI equals '/stream'.
 */
void websocket_open_cb(struct tcp_pcb *pcb, const char *uri) {
    printf("WS URI: %s\n", uri);
    if (!strcmp(uri, "/stream")) {
        printf("request for streaming\n");
        xTaskCreate(&websocket_task, "websocket_task", 1024, (void *) pcb, 2, NULL);
    }
}

void httpd_task(void *pvParameters) {
    tCGI pCGIs[] = {
        {"/", (tCGIHandler) websocket_cgi_handler},
    };

    const char *pcConfigSSITags[] = {
        "uptime", // SSI_UPTIME
        "heap",   // SSI_FREE_HEAP
        "led"     // SSI_LED_STATE
    };

    /* register handlers and start the server */
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0]));
    http_set_ssi_handler((tSSIHandler) ssi_handler, pcConfigSSITags,
            sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    websocket_register_callbacks((tWsOpenHandler) websocket_open_cb,
            (tWsHandler) websocket_cb);
    httpd_init();

    for (;;);
}

void serial_recv_task(void *pvParameters){
  thread_args_t * targs = (thread_args_t *) pvParameters;

  char buffer[100];
  int pos = 0;
  printf( "$");

	while(1){
    buffer[pos] = getchar();

    // If ENTER key is pressed, execute command
    if(buffer[pos] == UART_END){
      buffer[pos+1] = NULL;
      printf( "\r\n");
      tele_command_t command;
      // Populate the command structure with the data collected in buffer
      if(!parse_telemetry_string(&command, buffer)){
        printf("\rFailed to recognise string, ignoring\r\n");
      } else {
        // Populate our shared store of telemetry values
        memcpy(&targs->mbed_params[command.id], &command, sizeof(tele_command_t));
      }
      // This is a big dodgy but it works :)
      // We increment pos at the end of the loop so this -1 is only brief
      // TODO: Avoid this -1 set
      pos = -1;
    }

    if(buffer[pos] == UART_START){
      buffer[pos] = NULL;
      pos = 0;
    }

    buffer[pos+1] = NULL;
    printf("\r$ %s", buffer);
    pos++;
  }
}

void user_init(void) {
    uart_set_baud(0, 115200);

    printf("Triforce Telemetry v%s\n", VERSION);
    printf("FreeRTOS ESP8266 SDK v%s\n", sdk_system_get_sdk_version());
    printf("Heap Size: %d bytes\n", configTOTAL_HEAP_SIZE);

    struct sdk_station_config config = {
        .ssid = DEFAULT_WIFI_SSID,
        .password = DEFAULT_WIFI_PASS,
    };

    // Shared data between threads/tasks.
    thread_args_t targs;
    memset(&targs, 0x00, sizeof(thread_args_t));

    //Allow targs to be globally accessible
    gargs = &targs;

    printf("esp_init(): WiFi\r\n");
    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_connect();

    printf("esp_init(): Create Tasks\r\n");
    /* initialize tasks */
    xTaskCreate(&httpd_task, "HTTP Daemon", 128, (void *) &targs, 2, NULL);
    xTaskCreate(&serial_recv_task, "Serial Receive Task", 1024, (void*) &targs, 2, NULL);

    //Set ready pin to high
    gpio_enable(READY_PIN, GPIO_OUTPUT);
    gpio_write(READY_PIN, true);
}
