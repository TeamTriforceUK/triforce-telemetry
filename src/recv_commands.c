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

const char * recv_command_to_str(command_id_t id){
	if(id < 0 || id >= NUM_COMMANDS){
		return "INVALID COMMAND";
	}
	return recv_commands[id].name;
}

int command_generate(recv_command_t *command, char *buffer){
	printf("buffer: %s\r\n", buffer);
  //Get the size of the entire command string
  size_t command_len = strlen(buffer);
  char command_str[command_len];
  memcpy(command_str, buffer, command_len);

	// printf("command_string: %s (len: %d)\r\n", command_str, command_len);

  //Seperate commands into parts
  char param_part[10];
  char command_part[10];

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

      // Set command parameter for parameterised commands
			switch(command->type){
				  case CT_INT:
						command->param.i = atoi(param_part);
						printf("d: %d\r\n", command->param.i);
						break;
				  case CT_FLOAT:
						command->param.f = atof(param_part);
						printf("f: %f\r\n", command->param.f);
						break;
				  case CT_STRING:
					default:
						printf("unsupported param\r\n");
						break;
			}
      // int i;
      // for(i = 0; i < part; i++){
      //   memcpy(command->param[i], param_part[i], strlen(param_part[i]));
      // }

      //return true if matching command is found
      return RET_OK;
    }
  }

  //Return false (0) if no match is found
  return RET_ERROR;
}
