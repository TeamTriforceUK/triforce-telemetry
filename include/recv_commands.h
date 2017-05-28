/**
* @file recv_commands.h
* @author Cameron A. Craig
* @date 28 May 2017
* @copyright 2017 Cameron A. Craig
* @brief Defines commands that can be received on the serial port.
*/

#ifndef INCLUDE_RECV_COMMANDS_H_
#define INCLUDE_RECV_COMMANDS_H_

#include "recv_command_type.h"
#include "thread_args.h"

static recv_command_t recv_commands[] = {
  {.id = CID_RING_RPM, .name = "ring_rpm", .unit = CU_RPM, .type = CT_FLOAT},
  {.id = CID_CON_1_RPM, .name = "con_1_rpm", .unit = CU_RPM, .type = CT_FLOAT},
  {.id = CID_CON_2_RPM, .name = "con_2_rpm", .unit = CU_RPM, .type = CT_FLOAT},
  {.id = CID_ACCEL_X, .name = "accel_x", .unit = CU_MPSPS, .type = CT_FLOAT},
  {.id = CID_ACCEL_Y, .name = "accel_y", .unit = CU_MPSPS, .type = CT_FLOAT},
  {.id = CID_ACCEL_Z, .name = "accel_z", .unit = CU_MPSPS, .type = CT_FLOAT},
  {.id = CID_PITCH, .name = "pitch", .unit = CU_DEGREES, .type = CT_FLOAT},
  {.id = CID_ROLL, .name = "roll", .unit = CU_DEGREES, .type = CT_FLOAT},
  {.id = CID_YAW, .name = "yaw", .unit = CU_DEGREES, .type = CT_FLOAT},
  {.id = CID_WEAPON_VOLTAGE, .name = "w_voltage", .unit = CU_VOLTS, .type = CT_FLOAT},
  {.id = CID_DRIVE_VOLTAGE, .name = "d_voltage", .unit = CU_VOLTS, .type = CT_FLOAT},
  {.id = CID_AMBIENT_TEMP, .name = "temp", .unit = CU_CELCIUS, .type = CT_FLOAT},
  {.id = CID_ESP_LED, .name = "esp_led", .unit = CU_NONE, .type = CT_BOOLEAN},
};

#define NUM_COMMANDS (sizeof(recv_commands) / sizeof(recv_command_t))

const char * recv_command_to_str(command_id_t id);

int recv_command_generate(recv_command_t *command, char *buffer);
int recv_command_execute(recv_command_t *command, thread_args_t *targs);
  // switch (id) {
  //   case CID_RING_RPM:
  //   case CID_CON_1_RPM:
  //   case CID_CON_2_RPM:
  //   case CID_ACCEL_X:
  //   case CID_ACCEL_Y:
  //   case CID_ACCEL_Z:
  //   case CID_PITCH:
  //   case CID_ROLL:
  //   case CID_YAW:
  //   case CID_WEAPON_VOLTAGE:
  //   case CID_DRIVE_VOLTAGE:
  //   case CID_AMBIENT_TEMP:
  // }

// int process_command()

#endif  // INCLUDE_RECV_COMMANDS_H_
