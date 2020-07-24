#pragma once

#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float  float32;
typedef double float64;

typedef int32_t bool32;

void pop_error();

#if defined(DEBUG)
#define log_(message, ...) printf(message, ##__VA_ARGS__)
#define assert_(condition, message, ...) \
  if(!(condition)) { \
    log_(message, ##__VA_ARGS__); \
    log_("\n"); \
    pop_error(); \
    exit(1); \
  }
#else
#define log_(message, ...)
#define assert_(condition, message, ...)
#endif

void lock_mouse(bool32 confine);
void full_path(char* buffer, const char* fileName);
  
#include "Math.cpp"
#include "File.cpp"
#include "ImGUIImpl.cpp"

static uint16 windowWidth  = 960;   //@Note: These dont update. The mark the start size
static uint16 windowHeight = 540;  //@Note: These dont update. The mark the start size
#define ASPECT_RATIO (float32)windowWidth / (float32)windowHeight

//#include "OpenGLRenderer.cpp"
#include "D3DRenderer.cpp"
