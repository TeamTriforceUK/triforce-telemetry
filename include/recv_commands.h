/**
* @file recv_commands.h
* @author Cameron A. Craig
* @date 11 Nov 2017
* @copyright 2017 Cameron A. Craig
* @brief <brief>
*/

#include "return_codes.h"
#include "utils.h"
#include "thread_args.h"
#include "jsmn.h"
#include "tele_param.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int parse_telemetry_string(tele_command_t *command, char *buffer);
