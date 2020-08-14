#pragma once    

struct GameInput {
  bool32 up;
  bool32 left;
  bool32 down;
  bool32 right;
  bool32 close;
  bool32 quit;
  bool32 alt;
  bool32 editorMode;
  bool32 fire;

  float32 mouseWheel;
  Vec2 mousePosition;
  Vec2 rawMousePosition;
  bool32 mouseLocked;
};
