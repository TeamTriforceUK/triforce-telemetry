/**
* @file recv_commands.h
* @author Cameron A. Craig
* @date 25 May 2017
* @copyright 2017 Cameron A. Craig
* @brief Defines commands that can be received from the LPC1768.
*/
#ifndef INCLUDE_RECV_COMMAND_TYPE_H_
#define INCLUDE_RECV_COMMAND_TYPE_H_

#include <stdbool.h>

typedef enum  {
  CT_INT = 0,
  CT_FLOAT,
  CT_STRING,
	CT_BOOLEAN
} command_type_t;

typedef enum  {
  CU_RPM = 0,
  CU_RPS,
  CU_MPSPS,
  CU_CELCIUS,
  CU_VOLTS,
  CU_DEGREES,
	CU_NONE
} command_unit_t;

typedef enum  {
  CID_RING_RPM = 0,
  CID_CON_1_RPM,
  CID_CON_2_RPM,
  CID_ACCEL_X,
  CID_ACCEL_Y,
  CID_ACCEL_Z,
  CID_PITCH,
  CID_ROLL,
  CID_YAW,
  CID_WEAPON_VOLTAGE,
  CID_DRIVE_VOLTAGE,
  CID_AMBIENT_TEMP,
  CID_ESP_LED,
  CID_ARM_STATUS
} command_id_t;

typedef struct {
  command_id_t id;
  const char *name;
  command_unit_t unit;
  command_type_t type;

  union {
    float f;
    int i;
    char c;
    bool b;
  } param;

} recv_command_t;

#endif  // INCLUDE_RECV_COMMAND_TYPE_H_
