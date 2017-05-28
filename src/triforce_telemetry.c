/**
* @file http_server.c
* @author https://github.com/lujji, Cameron A. Craig
* @date 25 May 2017
* @copyright Public Domain
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
#include "recv_command_type.h"
#include "recv_commands.h"
#include "thread_args.h"

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

char *gpio_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "on") == 0) {
            uint8_t gpio_num = atoi(pcValue[i]);
            gpio_enable(gpio_num, GPIO_OUTPUT);
            gpio_write(gpio_num, true);
        } else if (strcmp(pcParam[i], "off") == 0) {
            uint8_t gpio_num = atoi(pcValue[i]);
            gpio_enable(gpio_num, GPIO_OUTPUT);
            gpio_write(gpio_num, false);
        } else if (strcmp(pcParam[i], "toggle") == 0) {
            uint8_t gpio_num = atoi(pcValue[i]);
            gpio_enable(gpio_num, GPIO_OUTPUT);
            gpio_toggle(gpio_num);
        }
    }
    return "/index.ssi";
}

char *about_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    return "/about.html";
}

char *websocket_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    return "/telemetry.html";
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
        int led = !gpio_read(LED_PIN);

        /* Generate response in JSON format */
        char response[512];
        int len = snprintf(response, sizeof (response),
          "{\"uptime\" : \"%d\","
          " \"heap\" : \"%d\","
          " \"led\" : \"%d\","
					" \"ring_rpm\" : \"%.2f\","
					" \"con_1_rpm\" : \"%.2f\","
					" \"con_2_rpm\" : \"%.2f\","
					" \"accel_x\" : \"%.2f\","
					" \"accel_y\" : \"%.2f\","
					" \"accel_z\" : \"%.2f\","
					" \"pitch\" : \"%.2f\","
					" \"roll\" : \"%.2f\","
					" \"yaw\" : \"%.2f\","
					" \"w_voltage\" : \"%.2f\","
					" \"d_voltage\" : \"%.2f\","
					" \"temp\" : \"%.2f\"}"
					, uptime, heap,
					gargs->esp_params.led,
					gargs->mbed_params.ring_rpm,
					gargs->mbed_params.con_1_rpm,
					gargs->mbed_params.con_2_rpm,
					gargs->mbed_params.accel_x,
					gargs->mbed_params.accel_y,
					gargs->mbed_params.accel_z,
					gargs->mbed_params.pitch,
					gargs->mbed_params.roll,
					gargs->mbed_params.yaw,
					gargs->mbed_params.weapon_voltage,
					gargs->mbed_params.drive_voltage,
					gargs->mbed_params.ambient_temp
				);
        if (len < sizeof (response))
            websocket_write(pcb, (unsigned char *) response, len, WS_TEXT_MODE);

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
            gargs->esp_params.led = false;
            val = 0xDEAD;
            break;
        case 'E': // Enable LED
            gargs->esp_params.led = true;
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
        {"/gpio", (tCGIHandler) gpio_cgi_handler},
        {"/about", (tCGIHandler) about_cgi_handler},
        {"/telemetry", (tCGIHandler) websocket_cgi_handler},
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
      recv_command_t command;
      //Generate a command structure for the command given
      if(!recv_command_generate(&command, buffer)){
        printf("\rCommand not recognised!\r\n");
      } else {
        printf("command: %s\r\n", recv_command_to_str(command.id));

				//Add command to queue
			// 	if(xQueueSendToBack(targs->command_queue, (void *) &command, (TickType_t) 10) != pdPASS){
			// 		printf("Could not add command to queue");
			// 	}
			recv_command_execute(&command, targs);
       };


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

void led_task(void *pvParameters){
	thread_args_t * targs = (thread_args_t *) pvParameters;
	while(1){
		gpio_write(LED_PIN, !targs->esp_params.led);
		vTaskDelay(50);
	}
}
// void command_exec_task(void *pvParameters){
// 	thread_args_t * targs = (thread_args_t *) pvParameters;
// 	recv_command_t command;
// 	while(1){
//
// 		if(xQueueReceive(targs->command_queue, &command, portMAX_DELAY) != pdPASS){
// 			printf("Nothing on queue!\r\n");
// 		} else {
// 			printf("Handling command\r\n");
// 			recv_command_execute(&command, targs);
// 		}
//
// 	}
// }


void user_init(void) {
    uart_set_baud(0, 115200);

    printf("Triforce Telemetry v%s\n", VERSION);
		printf("FreeRTOS ESP8266 SDK v%s\n", sdk_system_get_sdk_version());
		printf("Heap Size: %d bytes\n", configTOTAL_HEAP_SIZE);

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    // Shared data between threads/tasks.
		thread_args_t targs;
		memset(&targs, 0x00, sizeof(thread_args_t));

		//Allow targs to be globally accessible
		gargs = &targs;

		// printf("esp_init(): Command Queue\r\n");
		//Create command queue
		// targs.command_queue = xQueueCreate(10, sizeof(recv_command_t));
		// if(targs.command_queue == NULL){
		// 	printf("Command queue could not be created.\r\n");
		// }

		printf("esp_init(): WiFi\r\n");
    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_connect();

    /* turn off LED */
    gpio_enable(LED_PIN, GPIO_OUTPUT);
    gpio_write(LED_PIN, true);

		printf("esp_init(): Create Tasks\r\n");
    /* initialize tasks */
    xTaskCreate(&httpd_task, "HTTP Daemon", 128, (void *) &targs, 2, NULL);
    xTaskCreate(&led_task, "LED Update", 128, (void *) &targs, 2, NULL);
		xTaskCreate(&serial_recv_task, "Serial Receive Task", 1024, (void*) &targs, 2, NULL);
		// xTaskCreate(&command_exec_task, "Command Execute Task", 1024, (void*) &targs, 2, NULL);
}
