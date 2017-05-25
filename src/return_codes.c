/**
* @file return_codes.c
* @author Cameron A. Craig
* @date 18 May 2017
* @copyright 2017 Cameron A. Craig
* @brief Implements function definitons in return_codes.h
*/

#include "return_codes.h"

const char * err_to_str(int err){
  return ret_str[err];
}
