/**
* @file triforce_telemetry.c
* @author Cameron A. Craig
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
#include "FreeRTOS.h"
#include "semphr.h"
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
SemaphoreHandle_t shared_mutex;

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

    union  {
      bool b;
      int32_t i32;
      float f;
    } tmp_val;

    for (;;) {
        if (pcb == NULL || pcb->state != ESTABLISHED) {
            printf("Connection closed, deleting task\n");
            break;
        }

        int uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        int heap = (int) xPortGetFreeHeapSize();

        /* Generate response in JSON format */
        char response[MAX_JSON_STRLEN * 20];
        memset(&response[0], 0x00, MAX_JSON_STRLEN * 20);

        int i;
        int len = 0;

        /* We conveniently have two values not in the main array, so print one
           at the start and one at the end to handle the array opening and closing brackets.
        */
        len += snprintf(
          response + strlen(response),
          MAX_JSON_STRLEN,
          "[{\"name\": \"%s\", \"type\": \"%s\", \"unit\": \"%s\", \"value\": \"%df\"},\r",
          "uptime",
          "int32",
          "seconds",
          uptime
        );

        for(i = 0; i < gargs->num_mbed_params; i++) {
          /* All param strings start with the same string format */
          len += snprintf(
            response + strlen(response),
            MAX_JSON_STRLEN,
            "{\"name\": \"%s\", \"type\": \"%s\", \"unit\": \"%s\", \"value\": ",
            gargs->mbed_params[i].name,
            tele_command_type_to_string(gargs->mbed_params[i].type),
            tele_command_unit_to_string(gargs->mbed_params[i].unit)
          );

          switch (gargs->mbed_params[i].type) {
            case CT_FLOAT:
              /* Write to command safely */
              if(xSemaphoreTake(shared_mutex, (TickType_t) 10) == pdTRUE) {
                tmp_val.f = gargs->mbed_params[i].param.f;
                xSemaphoreGive(shared_mutex);
              } else {
                printf("%s: Could not take semaphore\r\n", __func__);
                break;
              }

              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "\"%.2f\"}\r",
                tmp_val.f
              );
            break;
            case CT_INT32:
              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "\"%d\"}\r",
                gargs->mbed_params[i].param.i32
              );
              break;
            case CT_BOOLEAN:
              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "\"%s\"}\r",
                gargs->mbed_params[i].param.b ? "ON" : "OFF"
              );
              break;
            case CT_NONE:
            default:
              len += snprintf(
                response + strlen(response),
                MAX_JSON_STRLEN,
                "\"%s\"}\r",
                "invalid"
              );
              break;
          }

          /* Add comma after each entry */
          len += snprintf(
            response + strlen(response),
            MAX_JSON_STRLEN,
            ","
          );
        }

        len += snprintf(
          response + strlen(response),
          MAX_JSON_STRLEN,
          "{\"name\": \"%s\", \"type\": \"%s\", \"unit\": \"%s\", \"value\": \"%df\"}]\r",
          "heap",
          "int32",
          "bytes",
          heap
        );

        if (len < sizeof (response)) {
          printf("Sending JSON data to websocket (len: %d)\r\n", len);
          websocket_write(pcb, (unsigned char *) response, len, WS_TEXT_MODE);
        } else {
          printf("Exceeded json string length budget!");
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

//static int parse_telemetry(thread_args_t *targs, char *buffer){}

void serial_recv_task(void *pvParameters){
  thread_args_t * targs = (thread_args_t *) pvParameters;

  char buffer[100];
  int pos = 0;
  tele_command_id_t id;

	while(1){
    buffer[pos] = getchar();

    // If ENTER key is pressed, execute command
    if(buffer[pos] == UART_END){
      buffer[pos+1] = NULL;
      printf( "\r\n");
      /* Get the ID first so we know where to store values.
      */
      if((id = parse_telemetry_string_id(buffer)) >= 0) {
        /* Get the type before we read a value,
           so we know how the value should be stored
        */
        if(parse_telemetry_string_type(targs, &targs->mbed_params[id], buffer) >= 0) {
          // Populate the command structure with the data collected in buffer
          if(parse_telemetry_string(targs, &targs->mbed_params[id], buffer) >= 0){
            printf("command set success\r\n");

            //If the command ID is the biggest so far, update
            if((id + 1) > targs->num_mbed_params) {
              targs->num_mbed_params = id + 1;
            }
          } else {
            printf("\rFailed to recognise string, ignoring\r\n");
          }
        }
      } else {
        printf("Failed to get ID\r\n");
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

    /* Initialise semaphores */
    targs.mutex.params = NULL;
    targs.mutex.params = xSemaphoreCreateMutex();
    if(targs.mutex.params != NULL) {
      printf("esp_init(): Create param semaphore.\r\n");
    } else {
      printf("Error: Failed to create semaphore!\r\n");
    }

    shared_mutex = NULL;
    shared_mutex = xSemaphoreCreateMutex();
    if(shared_mutex != NULL) {
      printf("esp_init(): Create shared_mutex semaphore.\r\n");
    } else {
      printf("Error: Failed to create shared_mutex semaphore!\r\n");
    }

    /* Make sure semaphores work */
    if(xSemaphoreTake(shared_mutex, (TickType_t) 10) == pdTRUE) {
      printf("esp_init: Lock obtained.\r\n");
      xSemaphoreGive(shared_mutex);
      printf("Unlocked\r\n");
    } else {
      printf("esp_init: Lock could not be obtained.\r\n");
    }


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
