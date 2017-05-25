/**
* @file utils.h
* @author Cameron A. Craig
* @date 18 May 2017
* @copyright 2017 Cameron A. Craig
* @brief Useful utility functions and macros.
*/


#ifndef TC_UTILS_H
#define TC_UTILS_H

#define MAX(a,b) \
  ({ typeof (a) _a = (a); \
     typeof (b) _b = (b); \
   _a > _b ? _a : _b; })

#define MIN(a,b) \
  ({ typeof (a) _a = (a); \
      typeof (b) _b = (b); \
    _a < _b ? _a : _b; })

#endif //TC_UTILS_H
