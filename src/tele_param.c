/* Copyright (c) 2017 Cameron A. Craig, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * @file tele_param.cpp
 * @author Cameron A. Craig
 * @date 10 Nov 2017
 * @copyright 2017 Cameron A. Craig
 * @brief String conversion functions for tele_param_*_t.
 */

#include "tele_param.h"
#include "stdint.h"
#include "string.h"

const char* tele_command_type_to_string(tele_command_type_t tctid){
  return tele_command_type_str[tctid];
}

const char* tele_command_unit_to_string(tele_command_unit_t tcuid){
  return tele_command_unit_str[tcuid];
}

tele_command_type_t string_to_type(char *string, size_t len) {
  int i;
  for (i = 0; i < NUM_CT; i++) {
    if(strncmp(string, tele_command_type_str[i], len) == 0) {
      return (tele_command_type_t) i;
    }
  }
  return CT_NONE;
}

tele_command_unit_t string_to_unit(char *string, size_t len) {
  int i;
  for (i = 0; i < NUM_CU; i++) {
    if(strncmp(string, tele_command_unit_str[i], len) == 0) {
      return (tele_command_unit_t) i;
    }
  }
  return CU_NONE;
}
