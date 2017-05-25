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

typedef struct {
  struct {
    float ring_rpm;
    float con_1_rpm;
    float con_2_rpm;
    float accel_x;
    float accel_y;
    float accel_z;
    float pitch;
    float roll;
    float yaw;
    float weapon_voltage;
    float drive_voltage;
    float ambient_temp;
  } mbed_params;

  QueueHandle_t command_queue;

} thread_args_t;
#endif  // INCLUDE_THREAD_ARGS_H_
