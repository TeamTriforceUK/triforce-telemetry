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

int parse_telemetry_string_id(char *buffer) {

  int ret;
  jsmn_parser parser;
  jsmntok_t t[24];

  jsmn_init(&parser);
  if((ret = jsmn_parse(&parser, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]))) < 0) {
    printf("%s: Failed to parse JSON: %d\n", __func__, ret);
    return -1;
  }

  /* Assume the top-level element is an object */
  if (ret < 1 || t[0].type != JSMN_OBJECT) {
    printf("%s: Object expected\n", __func__);
    return -1;
  }

  int i;
  for (i = 1; i < ret; i++) {
    if (jsoneq(buffer, &t[i], "id") == 0) {
      printf("id: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      // Put in NULL termination
      char tmp[t[i+1].end-t[i+1].start+1];
      strncpy(&tmp[0], buffer + t[i+1].start, t[i+1].end-t[i+1].start+1);
      return atoi(&tmp[0]);
    }
  }
  return -1;
}

int parse_telemetry_string_type(tele_command_t *command, char *buffer) {

  int ret;
  jsmn_parser parser;
  jsmntok_t t[24];

  jsmn_init(&parser);
  if((ret = jsmn_parse(&parser, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]))) < 0) {
    printf("%s: Failed to parse JSON: %d\n", __func__, ret);
    return -1;
  }

  /* Assume the top-level element is an object */
  if (ret < 1 || t[0].type != JSMN_OBJECT) {
    printf("%s: Object expected\n", __func__);
    return -1;
  }

  int i;
  for (i = 1; i < ret; i++) {
    if (jsoneq(buffer, &t[i], "type") == 0) {
      printf("type: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      command->type = string_to_type(buffer + t[i+1].start, t[i+1].end-t[i+1].start);
      printf("type id: %d\r\n", command->type);
      return 0;
    }
  }
  return -1;
}


int parse_telemetry_string(tele_command_t *command, char *buffer){

  printf("parsing: %s\r\n", buffer);
  int ret;
  jsmn_parser parser;
  jsmntok_t t[24];

  jsmn_init(&parser);
  if((ret = jsmn_parse(&parser, buffer, strlen(buffer), t, sizeof(t)/sizeof(t[0]))) < 0) {
    printf("%s: Failed to parse JSON: %d\n", __func__, ret);
    return -1;
  }

  /* Assume the top-level element is an object */
  if (ret < 1 || t[0].type != JSMN_OBJECT) {
    printf("%s: Object expected\n", __func__);
    return -1;
  }

  int i;
  for (i = 1; i < ret; i++) {
    if (jsoneq(buffer, &t[i], "name") == 0) {
      printf("name: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      memcpy(&command->name[0], buffer + t[i+1].start, t[i+1].end-t[i+1].start);
      i++;
    } else if (jsoneq(buffer, &t[i], "unit") == 0) {
      /* We may want to do strtol() here to get numeric value */
      printf("unit: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);
      command->unit = string_to_unit(buffer + t[i+1].start, t[i+1].end-t[i+1].start);
      printf("unit id: %d\r\n", command->unit);
      i++;
    } else if (jsoneq(buffer, &t[i], "value") == 0) {
      printf("value: %.*s\n", t[i+1].end-t[i+1].start,
      buffer + t[i+1].start);

      // add null termination
      char tmp[t[i+1].end-t[i+1].start+1];
      strncpy(&tmp[0], buffer + t[i+1].start, t[i+1].end-t[i+1].start+1);

      switch(command->type){
        case CT_FLOAT:
          command->param.f = atof(&tmp[0]);
          break;
        case CT_INT32:
          command->param.i32 = atoi(&tmp[0]);
          break;
        case CT_BOOLEAN:
          //TODO: Implement!
          command->param.b = false;
          break;
        case CT_STRING:
          strncpy(&command->param.s[0], &tmp[0], t[i+1].end-t[i+1].start);
          break;
        default:
          printf("Cannot parse type %d from mbed!\r\n", command->type);
          break;
      }
    }
  }
  return 0;
}
