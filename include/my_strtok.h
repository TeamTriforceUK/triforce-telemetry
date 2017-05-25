/**
* @file my_strtok.h
* @author Cameron A. Craig
* @date 25 May 2017
* @copyright 2017 Cameron A. Craig
* @brief Custom implementation of strtok, have a feeling strtok() has issues.
*
*/
#ifndef INCLUDE_MY_STRTOK_H_
#define INCLUDE_MY_STRTOK_H_

char * my_strtok(char *str, const char *delim) {
  static char *str_begin;
  if (str != NULL) {
    str_begin = str;
  }else{
		
	}

char * tmp = str_begin;
int i;
for(tmp = str; *tmp != NULL ; tmp++) {
	if(*tmp == *delim) {
		*tmp = NULL;
	}
}
return
}

#endif  // INCLUDE_MY_STRTOK_H_
