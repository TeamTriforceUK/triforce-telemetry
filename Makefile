PROGRAM=triforce_telemetry

EXTRA_CFLAGS=-DLWIP_HTTPD_CGI=1 -DLWIP_HTTPD_SSI=1 -I./fsdata -I./include -L./libs/jsmn -ljsmn
ESP_OPEN_RTOS_PATH=/home/cameron/git/esp-open-rtos
PROGRAM_SRC_DIR=. ./src
PROGRAM_INC_DIR=. ./include ./libs/jsmn
#Enable debugging
#EXTRA_CFLAGS+=-DLWIP_DEBUG=1 -DHTTPD_DEBUG=LWIP_DBG_ON

EXTRA_COMPONENTS=$(ESP_OPEN_RTOS_PATH)/extras/mbedtls $(ESP_OPEN_RTOS_PATH)/extras/httpd

include $(ESP_OPEN_RTOS_PATH)/common.mk

html:
	@echo "Generating fsdata.."
	cd frontend && ./makefsdata
	mv frontend/fsdata.c fsdata.c

cleanfs:
	rm fsdata.c
