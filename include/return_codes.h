/**
* @file return_codes.h
* @author Cameron A. Craig
* @date 18 May 2017
* @copyright 2017 Cameron A. Craig
* @brief Defines return codes that are used to propogate errors to the user.
*/

#ifndef TC_RETURN_CODES_H
#define TC_RETURN_CODES_H

typedef enum {
  RET_ERROR = 0,
  RET_OK,
  RET_ALREADY_DISARMED,
  RET_ALREADY_ARMED
} return_codes_t ;

static const char * ret_str[] = {
  "Error",
  "Ok",
  "Already disarmed",
  "Already armed"
};

const char * err_to_str(int err);

#endif //TC_RETURN_CODES_H
