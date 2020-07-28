#pragma once
#include <string.h>

const uint32 MAX_STR_LEN = 256;

struct String {
  char str[MAX_STR_LEN];

  String() {}

  String(const char* ptr) {
    assert_(strlen(ptr) <= MAX_STR_LEN, "String length can't be longer than %d characters", MAX_STR_LEN);
    strcpy(str, ptr);
  }
  
  String& operator=(const char* ptr) {
    assert_(strlen(ptr) <= MAX_STR_LEN, "String length can't be longer than %d characters", MAX_STR_LEN);
    strcpy(str, ptr);
    return *this;
  }
};
