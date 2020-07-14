#pragma once

#define kilobytes(value) (value * 1024LL) 
#define megabytes(value) (kilobytes(value) * 1024LL) 
#define gigabytes(value) (megabytes(value) * 1024LL) 

struct GameMemory {
  void* memory = nullptr;
  uint64 size = 0;
  uint32 offset = 0;
  bool32 isInitialized = false;
};
