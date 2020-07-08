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

void* reserve(GameMemory* memory, uint32 amt) {
  uint32 index = memory->offset + amt;
  assert(index <= memory->size, "Allocating more memory than the game allows! Terminating..");
  memory->offset += amt;
  return ((uint8*)memory->memory + index); 
}
