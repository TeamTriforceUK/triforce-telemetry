/**
* @file recv_commands.c
* @author Cameron A. Craig
* @date 25 May 2017
* @copyright 2017 Cameron A. Craig
* @brief <brief>
*/

#include "recv_commands.h"
#include "return_codes.h"
#include "utils.h"
#include "thread_args.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const char * recv_command_to_str(command_id_t id){
	if(id < 0 || id >= NUM_COMMANDS){
		return "INVALID COMMAND";
	}
	return recv_commands[id].name;
}

int recv_command_generate(recv_command_t *command, char *buffer){
	printf("buffer: %s\r\n", buffer);
  //Get the size of the entire command string
  size_t command_len = strlen(buffer);
  char command_str[command_len];
  memcpy(command_str, buffer, command_len);

	// printf("command_string: %s (len: %d)\r\n", command_str, command_len);

  //Seperate commands into parts
	/* TODO: command_len is a safe size for these buffers, but we could be smart and
	define these after we have tokenised, and use the correct size buffer. */
  char param_part[command_len];
  char command_part[command_len];

  char *token;
	char *saveptr;
	char * str;

  int j;
	for(j = 1,str = command_str;; j++, str = NULL) {
		token = strtok_r(str, " ", &saveptr);
		if(token == NULL){
			break;
		}
		// printf("%d, %s\r\n", j,token);
	  if(j == 1){
      strncpy(command_part, token, strlen(token));
    } else {
      strncpy(param_part, token, strlen(token));
    }
  }

  //Find a matching command for the given command string
  int i;
  for(i = 0; i < NUM_COMMANDS; i++){
    size_t cplen = strlen(command_part);
    char *command_compare;
    command_compare = (char *) recv_command_to_str(recv_commands[i].id);
    size_t cclen = strlen(command_compare);

		// printf("comparing %s and %s\r\n", command_compare, command_part);

    if(strncmp(command_compare, command_part, MIN(cclen, cplen)) == 0){
      //When a matching command is found, populate the command
      command->id = recv_commands[i].id;
      command->name = recv_commands[i].name;
      command->type = recv_commands[i].type;
      command->unit = recv_commands[i].unit;

			// printf("param_part: %s\r\n", param_part);

      char *end;
			int t;
			size_t param_len = strlen(param_part);
			printf("param len: %d\r\n", param_len);

      // Set command parameter for parameterised commands
			switch(command->type){
				  case CT_INT32:
						command->param.i = strtol(param_part, &end, 10);
						if(end == param_part){
							printf("Conversion error\r\n");
							return RET_ERROR;
						}
						printf("d: %d\r\n", command->param.i);
						break;
				  case CT_FLOAT:
						command->param.f = strtod(param_part, &end);
						if(end == param_part){
							printf("Conversion error\r\n");
							return RET_ERROR;
						}
						printf("f: %f\r\n", command->param.f);
						break;
					case CT_BOOLEAN:
						command->param.b = (strtod(param_part, &end) == 1);
						if(param_part == end){
							return RET_ERROR;
						}
						return RET_OK;
				  case CT_STRING:
					default:
						printf("unsupported param\r\n");
						return RET_ERROR;
			}

      //return true if matching command is found
      return RET_OK;
    }
  }

  //Return false (0) if no match is found
  return RET_ERROR;
}

int recv_command_execute(recv_command_t *command, thread_args_t *targs){
  // TODO: Do a bit of refactoring to use type in switch, rather than ID
  switch(command->id){
    case CID_DRIVE_1_RPM:
      targs->mbed_params.drive_1_rpm = command->param.f;
      break;
    case CID_DRIVE_2_RPM:
      targs->mbed_params.drive_2_rpm = command->param.f;
      break;
    case CID_DRIVE_3_RPM:
      targs->mbed_params.drive_3_rpm = command->param.f;
      break;
    case CID_WEAPON_1_RPM:
      targs->mbed_params.weapon_1_rpm = command->param.f;
      break;
    case CID_WEAPON_2_RPM:
      targs->mbed_params.weapon_2_rpm = command->param.f;
      break;
    case CID_WEAPON_3_RPM:
      targs->mbed_params.weapon_3_rpm = command->param.f;
      break;
    case CID_ACCEL_X:
      targs->mbed_params.accel_x = command->param.f;
      break;
    case CID_ACCEL_Y:
      targs->mbed_params.accel_y = command->param.f;
      break;
    case CID_ACCEL_Z:
      targs->mbed_params.accel_z = command->param.f;
      break;
    case CID_PITCH:
      targs->mbed_params.pitch = command->param.f;
      break;
    case CID_ROLL:
      targs->mbed_params.roll = command->param.f;
      break;
    case CID_YAW:
      targs->mbed_params.yaw = command->param.f;
      break;
    case CID_WEAPON_VOLTAGE:
      targs->mbed_params.weapon_voltage = command->param.f;
      break;
    case CID_DRIVE_VOLTAGE:
      targs->mbed_params.drive_voltage = command->param.f;
      break;
    case CID_AMBIENT_TEMP:
      targs->mbed_params.ambient_temp = command->param.f;
      break;
    case CID_ESP_LED:
      targs->esp_params.led = command->param.b;
      printf("LED now %s\r\n", command->param.b ? "On" : "Off");
      break;
    case CID_ARM_STATUS:
      targs->mbed_params.arm_status = (state_t) command->param.i;
    break;
  }
}
