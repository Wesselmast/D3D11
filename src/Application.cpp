#pragma once
#include <vector>

#include "Camera.cpp"
#include "GameCollision.cpp"
#include "GameLogic.cpp"
#include "PHP.cpp"

struct GameInput {
  bool32 up;
  bool32 left;
  bool32 down;
  bool32 right;
  bool32 close;
  bool32 quit;
  bool32 alt;
  bool32 shift;
  bool32 editorMode;
  bool32 fire;
  bool32 click;

  float32 mouseWheel;
  Vec2 mousePosition;
  Vec2 rawMousePosition;
  bool32 mouseLocked;
};

struct GameState {
  RenderObjects renderObjects;
  Cameras cameras;

  PHP php;
  AccountInfo accountInfo;

  uint32 editorCamera;
  uint32 gameCamera;
  Vec3 cameraPos;
  Vec3 cameraRot;
  Vec2 lastMousePosition;
  float32 cameraSpeed;

  bool32 editorMode;
  bool32 lastEditorMode;

  bool32 anySelected;
  uint32 selected;

  PointLight light;
  Vec3 lightPos;

  Transform startTransform;
  uint32 player;
  Transform playerTransform;
  
  float32 playerSpeed;  
  float32 playerWalkSpeed;  
  float32 playerRunSpeed;  
  
  std::vector<Bullet> bullets;
  float32 fireInterval;
  uint32 maxBullets;
};

const uint32 AMOUNT_OF_TEXTURES = 20;
const uint32 AMOUNT_OF_MODELS = 20;

static Texture textures[AMOUNT_OF_TEXTURES];
static ModelInfo models[AMOUNT_OF_MODELS];

#if defined(NETWORKING)
#include "Networking.cpp"
#include "Server.cpp"
#include "Client.cpp"
#endif

#if defined(NETWORKING)
static bool32 isServer;
static bool32 isClient;
static char targetConnectionIP[128];
static int32 targetConnectionPort;
#endif

#include "Game.cpp"
#include "Editor.cpp"

void import_texture(GameMemory* memory, Texture* textures, const char* path, uint32 index) {
  Bitmap bmp = {};
  load_bitmap(memory, path, bmp);
  textures[index] = make_texture(bmp);
}

void import_model(GameMemory* memory, ModelInfo* models, const char* path, uint32 index) {
  ModelInfo info = {};
  load_obj(memory, path, info);
  models[index] = info;
}

int32 app_update(GameMemory* memory, GameInput* input, float64 dt, float64 time) {
  GameState* state  = (GameState*)memory->memory;
  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);
  
  if(!memory->isInitialized) {
    test_renderer();

    initialize();
    state->accountInfo = {};
    php_connect(&(state->php));

    memory->offset = sizeof(GameState);
    assert_(memory->offset <= memory->size, 
	    "Too little memory is used! %llu is needed and %llu is available!", 
	    memory->offset, memory->size);
    state->editorCamera = create_camera(cameras);
    state->gameCamera = create_camera(cameras);
    state->light = {};
    state->light.camera = create_camera(cameras);

#if defined(NETWORKING)
    isClient = 0;
    isServer = 0;
#endif

    import_texture(memory, textures, "res\\textures\\T_Pixel.bmp",        0);
    import_texture(memory, textures, "res\\textures\\T_CheckerBoard.bmp", 1);
    import_texture(memory, textures, "res\\textures\\T_Creature4.bmp",    2);
    import_texture(memory, textures, "res\\textures\\T_Brick.bmp",        3);
    import_texture(memory, textures, "res\\textures\\T_Desert.bmp",       4);
    import_texture(memory, textures, "res\\textures\\T_CowboyOutfit.bmp", 5);

    uint32 mIndex = 0;
    import_model(memory, models, "res\\models\\Cube.obj",      0);
    import_model(memory, models, "res\\models\\Plane.obj",     1);
    import_model(memory, models, "res\\models\\Monkey.obj",    2);
    import_model(memory, models, "res\\models\\Cactus.obj",    3);
    import_model(memory, models, "res\\models\\Sphere.obj",    4);
    import_model(memory, models, "res\\models\\Creature.obj",  5);
    import_model(memory, models, "res\\models\\Cowboy.obj",    6);

    state->player = create_player(ro, models, false);    
    memory->isInitialized = true;

    load_level(memory, ro, models, "res\\levels\\try.level");
    game_start(state);
  }
  
#if defined(NETWORKING)
  if(isServer) server_update(state);
  if(isClient) {
    if(!client_update(state)) {
      client_disconnect();
    }
  }
#endif
  
  uint32 res;
  if(state->editorMode) {
    res = editor_update(state, input, dt, time);
    render_loop(get_view_projection(cameras, state->editorCamera), &(state->light));
  }
  else {
    res = game_update(state, input, dt, time);
    render_loop(get_view_projection(cameras, state->gameCamera), &(state->light));
  }
  update_render_objects(ro, textures);

  start_ImGUI();
  if(state->editorMode) {
    render_editor_ui(memory, state);
  }
  else {
    render_game_ui(memory, state);
  }
  end_ImGUI();

  if(input->editorMode != state->lastEditorMode && input->editorMode) {
    state->editorMode = !state->editorMode;
    if(!state->editorMode) {
      reset_player_position(state);      
    }
  }

  state->lastEditorMode = input->editorMode;
  state->lastMousePosition = input->rawMousePosition;
  input->click = false;

  if(!res) { 
    php_disconnect(&(state->php));
  }
  return res;
}

void app_quit() {
#if defined(NETWORKING)
  if(isServer) server_shutdown(); 
  if(isClient) client_disconnect();
  shutdown();
#endif
}

