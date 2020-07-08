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
  memory->offset += amt;
  assert(memory->offset <= memory->size, "Allocating more memory than the game allows! Terminating..");
  return (((uint8*)(memory->memory)) + memory->offset); 
}
