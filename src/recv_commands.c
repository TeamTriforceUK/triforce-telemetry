/**
* @file recv_commands.c
* @author Cameron A. Craig
* @date 25 May 2017
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

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

// const char * tele_command_to_str(tele_command_id_t id){
// 	if(id < 0 || id >= NUM_COMMANDS){
// 		return "INVALID COMMAND";
// 	}
// 	return recv_commands[id].name;
// }

int parse_telemetry_string(tele_command_t *command, char *buffer){
  int ret;
  jsmn_parser parser;
  jsmntok_t t[24];

  jsmn_init(&parser);
  if((ret = jsmn_parse(&parser, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]))) < 0) {
    printf("Failed to parse JSON: %d\n", ret);
    return 1;
  }

  /* Assume the top-level element is an object */
  if (ret < 1 || t[0].type != JSMN_OBJECT) {
    printf("Object expected\n");
    return 1;
  }

  int i;
  for (i = 1; i < ret; i++) {
    if (jsoneq(buffer, &t[i], "name") == 0) {
      /* We may use strndup() to fetch string value */
      printf("name: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      i++;
    } else if (jsoneq(buffer, &t[i], "type") == 0) {
      /* We may additionally check if the value is either "true" or "false" */
      printf("type: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      i++;
    } else if (jsoneq(buffer, &t[i], "unit") == 0) {
      /* We may want to do strtol() here to get numeric value */
      printf("unit: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      i++;
    } else if (jsoneq(buffer, &t[i], "value") == 0) {
      printf("value: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
    }
  }
}
