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
#include "tele_param.h"

typedef struct {
  tele_command_t mbed_params[30];
  uint32_t num_mbed_params;
} thread_args_t;
#endif  // INCLUDE_THREAD_ARGS_H_
