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

struct GameInput {
  bool32 up;
  bool32 left;
  bool32 down;
  bool32 right;
  bool32 quit;

  Vec2 mousePosition;
  Vec2 rawMousePosition;
  bool32 mouseLocked;
};
