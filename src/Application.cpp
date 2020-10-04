#pragma once
#include <vector>

#include "Camera.cpp"
#include "GameCollision.cpp"
#include "GameLogic.cpp"
#include "PHP.cpp"

#if defined(NETWORKING)
#include "Networking.cpp"
#endif

enum GameResult {
  NONE,
  GAME_WON,
  GAME_LOST
};

struct GameInput {
  bool32 up;
  bool32 left;
  bool32 down;
  bool32 right;
  bool32 pauseMenu;
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
  int32 investment;
  
  std::vector<Bullet> bullets;
  float32 fireInterval;
  uint32 maxBullets;

  int32 hitpoints;

  bool32 showMainMenu;
  bool32 showPauseMenu;
  bool32 showNetworking;

  bool32 startGame;
  bool32 playGame;

  bool32 practiceMode;
  bool32 networkMode;

  bool32 gameOver;
  GameResult gameResult;

#if defined(NETWORKING)
  Client client;
  Server server;
#endif
};

const uint32 AMOUNT_OF_TEXTURES = 20;
const uint32 AMOUNT_OF_MODELS = 20;
static Texture textures[AMOUNT_OF_TEXTURES];
static ModelInfo models[AMOUNT_OF_MODELS];

#include "Game.cpp"
#include "Editor.cpp"

#ifdef ASYNC
#include <future>
static std::mutex mutex;
#endif

static void import_texture(GameMemory* memory, Texture* textures, const char* path, uint32 index) {
  Bitmap bmp = {};
  load_bitmap(memory, path, bmp);
#ifdef ASYNC
  std::lock_guard<std::mutex> lock(mutex);
#endif
  textures[index] = make_texture(bmp);
}

static void import_model(GameMemory* memory, ModelInfo* models, const char* path, uint32 index) {
  ModelInfo info = {};
  load_obj(memory, path, info);
#ifdef ASYNC
  std::lock_guard<std::mutex> lock(mutex);
#endif
  models[index] = info;
}

static void load_all_objects(GameMemory* memory, Texture* textures, ModelInfo* models, const char* path) {
  std::chrono::high_resolution_clock timer;
  auto start = timer.now();

  char texturePaths[AMOUNT_OF_TEXTURES][256];
  char modelPaths[AMOUNT_OF_MODELS][256];
  uint32 amountOfTextures = 0;
  uint32 amountOfModels = 0;
  load_object_paths(path, texturePaths, modelPaths, amountOfTextures, amountOfModels); 
#ifdef ASYNC
  std::future<void> futures[amountOfTextures + amountOfModels];
#endif  
  for(uint32 i = 0; i < amountOfTextures; i++) {
#ifdef ASYNC
    futures[i] = std::async(std::launch::async, import_texture, 
			    memory, textures, texturePaths[i], i);
#else
    import_texture(memory, textures, texturePaths[i], i);
#endif
    
  }
  for(uint32 i = 0; i < amountOfModels; i++) {
#ifdef ASYNC
    uint32 fi = i + amountOfTextures; 
    futures[fi] = std::async(std::launch::async, import_model, 
			     memory, models, modelPaths[i], i);
#else
    import_model(memory, models, modelPaths[i], i);
#endif
  }
   
#ifdef ASYNC
  for(std::future<void>& f : futures) {
    f.wait();
  } 
#endif

  log_("Total time taken loading models : %f seconds\n", 
       std::chrono::duration<float64>(timer.now() - start).count());
}

int32 app_update(GameMemory* memory, GameInput* input, float64 dt, float64 time) {
  GameState* state  = (GameState*)memory->memory;
  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);
  
  if(!memory->isInitialized) {
    test_renderer();

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
    state->investment = 1000;

#if defined(NETWORKING)
    initialize();
    state->client = {};
    state->server = {};
#endif

    load_all_objects(memory, textures, models, "res\\.objectpaths");

    state->player = create_player(ro, models, false);    
    memory->isInitialized = true;

    load_level(memory, ro, models, "res\\levels\\try.level");
  }
  
  uint32 res = 0;
  if(state->editorMode) {
    res |= editor_update(state, input, dt, time);
    render_loop(get_view_projection(cameras, state->editorCamera), &(state->light));
  }

  else {
    res |= game_update(state, input, dt, time);
    render_loop(get_view_projection(cameras, state->gameCamera), &(state->light));
  }
  update_render_objects(ro, textures);

  start_ImGUI();
  if(state->editorMode) {
    res |= render_editor_ui(memory, state);
  }
  else {
    res |= render_game_ui(memory, state);
  }
  end_ImGUI();

  if(input->editorMode != state->lastEditorMode && input->editorMode) {
    state->editorMode = !state->editorMode;
    if(!state->editorMode) {
      state->playerTransform = state->startTransform;
    }
  }

  state->lastEditorMode = input->editorMode;
  state->lastMousePosition = input->rawMousePosition;
  input->click = false;

  res |= input->quit;

  if(res) { 
    php_disconnect(&(state->php));
#if defined(NETWORKING)
    if(state->server.valid) server_shutdown(&state->server, ro); 
    if(state->client.valid) client_disconnect(&state->client, ro);
#endif
  }
  return res;
}

void app_quit() {
#if defined(NETWORKING)
  shutdown();
#endif
}

