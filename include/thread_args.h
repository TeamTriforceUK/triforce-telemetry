/**
* @file thread_args.h
* @author Cameron A. Craig
* @date 25 May 2017
* @copyright 2017 Cameron A. Craig
* @brief <brief>
*/

#ifndef INCLUDE_THREAD_ARGS_H_
#define INCLUDE_THREAD_ARGS_H_

#include <FreeRTOS.h>
#include <queue.h>

#include "states.h"

typedef struct {
  struct {
    int32_t drive_1_rpm;
    int32_t drive_2_rpm;
    int32_t drive_3_rpm;
    int32_t weapon_1_rpm;
    int32_t weapon_2_rpm;
    int32_t weapon_3_rpm;
    float accel_x;
    float accel_y;
    float accel_z;
    float pitch;
    float roll;
    float yaw;
    float weapon_voltage;
    float drive_voltage;
    float ambient_temp;
    state_t arm_status
  } mbed_params;

  struct {
    bool led;
  } esp_params;

  QueueHandle_t command_queue;

} thread_args_t;
#endif  // INCLUDE_THREAD_ARGS_H_
