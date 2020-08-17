#include "Game.h"
#include "Camera.cpp"
#include "GameCollision.cpp"
#include "SQLServer.cpp"
#include "Config.cpp"
#include "GameLogic.cpp"

struct GameState {
  RenderObjects renderObjects;
  Cameras cameras;

  SQLInfo sqlInfo;
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
  
  std::vector<Bullet> bullets;
  float32 fireInterval;
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

void reset_player_position(GameState* state) {
  state->playerTransform = state->startTransform;
}

void game_start(GameState* state) {
  RenderObjects* ro = &(state->renderObjects);

  state->startTransform = {{ 0.0f, -7.2f, 0.0f }, vec3_from_scalar(0.0f)};
  state->playerTransform = state->startTransform; 
  state->playerSpeed = 8.0f;

  set_object_transform(ro, state->player, state->playerTransform);
}

void render_imgui(GameMemory* memory, GameState* state) {
  RenderObjects* ro = &(state->renderObjects);

  start_ImGUI();

  if(!state->accountInfo.loggedin) {
    const ImVec2 nws = { 300, 175 };
    ImGui::SetNextWindowPos(ImVec2((windowWidth  * 0.5f) - (nws.x * 0.5f), 
				   (windowHeight * 0.5f) - (nws.y * 0.5f)));
    ImGui::SetNextWindowSize(nws);

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("Account", 0, flags);
    ImGui::Text("Welcome to *Untitled*"); 
    ImGui::Text("please log in or register to continue!");
    ImGui::NewLine();
    ImGui::InputText("* Username", (char*)&(state->accountInfo.username), 128);
    ImGui::InputText("* Password", (char*)&(state->accountInfo.password), 128, ImGuiInputTextFlags_Password);

    if(ImGui::Button("Register")) {
      if(register_account(state->accountInfo, &(state->sqlInfo))) {
	log_("successfully registered!\n");
      }
    }

    if(ImGui::Button("Login")) {
      if(login_account(state->accountInfo, &(state->sqlInfo))) {
	log_("successfully logged in!\n");
	state->accountInfo.loggedin = true;
      }
    }

    ImGui::End();

    end_ImGUI();
    return;
  }

  uint32 obj;
  if(state->anySelected) {
    ObjectDescriptor* descPtr = get_object_descriptor(ro, state->selected); 
    ImGui::Begin(descPtr->name);
    
    bool changed = false;

    ImGui::Text("Position");
    if(ImGui::DragFloat3("pos", ((float32*)(&(descPtr->transform.position))), 0.4f, -100.0f, 100.0f)) {
      changed = true;
    }
    Vec3 tempEulerRot = descPtr->transform.rotation * radians_to_degrees();
    if(ImGui::DragFloat3("rot", ((float32*)&tempEulerRot), 0.4f, -360.0f, 360.0f)) {
      descPtr->transform.rotation = tempEulerRot * degrees_to_radians();  
      changed = true;
    }
    if(ImGui::DragFloat3("scl", ((float32*)(&(descPtr->transform.scale   ))), 0.4f, -50.0f,  50.0f)) {
      changed = true;
    }
    ImGui::NewLine();
    ImGui::Text("Material color");
    if(ImGui::DragFloat3("mat", ((float32*)(&(descPtr->material.materialColor))), 0.02f, 0.0f, 2.0f)) {
      changed = true;
    }
    ImGui::NewLine();
    ImGui::Text("Texture tiling");
    if(ImGui::DragFloat2("til", ((float32*)(&(descPtr->material.tiling))), 0.4f, 0.01f, 20.0f)) {
      changed = true;
    }
    ImGui::NewLine();
    
    ImGui::Image(textures[descPtr->textureRef].resource, ImVec2(128, 128));
        
    if(ImGui::Button("Texture")) {
      ImGui::OpenPopup("Texture FileList");
    }
    if(ImGui::BeginPopup("Texture FileList")) {
      for(uint32 i = 0; i < AMOUNT_OF_TEXTURES; i++) {
	if(!textures[i].resource) break;
	ImGui::SameLine();
	if(ImGui::ImageButton(textures[i].resource, ImVec2(64, 64))) {
	  descPtr->textureRef = i;
	  changed = true;
	  ImGui::CloseCurrentPopup();
	  break;
	}
	if(i % 2 == 0 && i != 0) {
	  ImGui::NewLine();
	}
      }
      ImGui::EndPopup();
    }    

    if(ImGui::Button("Model")) {
      ImGui::OpenPopup("Model FileList");
    }
    if(ImGui::BeginPopup("Model FileList")) {
      for(uint32 i = 0; i < AMOUNT_OF_MODELS; i++) {
	if(!models[i].vSize) break;
	ImGui::SameLine();
	if(ImGui::Button(models[i].name)) {
	  descPtr->modelRef = i;
	  ObjectDescriptor newDesc;
	  memcpy(&newDesc, descPtr, sizeof(ObjectDescriptor));

	  state->anySelected = false;
	  destroy_object(ro, state->selected);

	  uint32 newObj = create_object_from_index(ro, &newDesc, models, state->selected);
	  changed = true;
	  ImGui::CloseCurrentPopup();
	  break;
	}
	if(i % 2 == 0 && i != 0) {
	  ImGui::NewLine();
	}
      }
      ImGui::EndPopup();
    }

    if(changed) {
      set_object_descriptor(ro, state->selected, descPtr);
    }
    if(ImGui::Button("Copy selected")) {
      create_object(ro, descPtr, models);
    }
    if(ImGui::Button("Destroy selected")) {
      state->anySelected = false;
      destroy_object(ro, state->selected);
    }

    ImGui::End();
  }
    
  ImGui::Begin("World manager");

  bool open_spawn_popup = ImGui::Button("Spawn");
  if(open_spawn_popup) {
    ImGui::OpenPopup("SpawnWindow");
  }
  
  if(ImGui::BeginPopup("SpawnWindow")) {
    for(uint32 i = 0; i < AMOUNT_OF_MODELS; i++) {
      if(!models[i].vertices) break;
      ImGui::SameLine();
      if(ImGui::Button(models[i].name)) {
	create_model(ro, models, i);
	break;
	ImGui::CloseCurrentPopup();
      }
      if(i % 2 == 0 && i != 0) {
	ImGui::NewLine();
      }
    }
    ImGui::EndPopup();
  }
  if(ImGui::Button("Save level")) {
    save_level(ro, "res\\levels\\try.level");
  }
  if(ImGui::Button("Load level")) {
    state->anySelected = false;
    load_level(memory, ro, models, "res\\levels\\try.level");
    game_start(state);
  }

  ImGui::End();

  {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(250, 120));

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("Slot machine!", 0, flags);
    ImGui::Text("Welcome, %s", state->accountInfo.username);

    uint32 gamble = 2500;
    if(ImGui::Button("Spend 2500 to spin the wheel!")) {
      int64 score = atol(state->accountInfo.score);

      if(score - gamble >= 0) {
	score -= gamble;
	
	//what's it gonna be?!
	score += rand() % gamble * 2;
	
	_ltoa(score, state->accountInfo.score, 10);
	
	if(!update_account(state->accountInfo, &(state->sqlInfo))) {
	  log_("Couldn't update the score for you. Something went wrong!\n");
	}
      }
      else {
	log_("You don't have enough money to spin! D:\n");
      }
    }

    ImGui::Text("Score: %s", state->accountInfo.score);

    ImGui::NewLine();
    if(ImGui::Button("Log out")) {
      state->accountInfo = {};
      state->accountInfo.loggedin = false;
    } 

    ImGui::End();
  } 
 
  {
    ImGui::Begin("Light");
    
    ImGui::Text("Position");
    if(ImGui::DragFloat3("pos", ((float32*)(&state->lightPos)), 0.4f, -100.0f, 100.0f)) {
      state->light.lightBuffer.position = state->lightPos;
      set_camera_transform(&(state->cameras), state->light.camera, state->lightPos, vec3_from_scalar(0.0f));
    }
    
    ImGui::End();
  }

#if defined(NETWORKING)
  ImGui::Begin("Networking");
  
  ImGui::InputText("ip", targetConnectionIP, 128);
  ImGui::SameLine();
  ImGui::InputInt("port", &targetConnectionPort);

  if(ImGui::Button("Start server")) {
    if(isServer) server_shutdown();
    if(isClient) client_disconnect();
    isServer = 1;
    isClient = 0;    
    server_startup(create_ip_endpoint(targetConnectionIP, (uint16)targetConnectionPort));
  }
  if(ImGui::Button("Start client")) {
    if(isServer) server_shutdown();
    if(isClient) client_disconnect();
    isServer = 0;
    isClient = 1;    
    client_connect(create_ip_endpoint(targetConnectionIP, (uint16)targetConnectionPort));
  }  
  ImGui::End();
#endif

  end_ImGUI();
}

uint32 game_mode(GameState* state, GameInput* input, float64 dt, float64 time) {
  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);
  
  if(input->mouseLocked) lock_mouse(false);

  Transform& transform = state->playerTransform;

  Vec3 nextMove = {};
  nextMove.z = input->up    - input->down;
  nextMove.x = input->right - input->left;
  nextMove = vec3_normalize(nextMove);

  transform.position.x += nextMove.x * dt * state->playerSpeed;
  transform.position.z += nextMove.z * dt * state->playerSpeed;

  Vec3 world = screen_to_world(cameras, state->gameCamera, input->mousePosition); 
  world = world * 20.0f + state->cameraPos;
  Vec2 world2d = { world.x, world.z };
  Vec2 pos2d = { transform.position.x, transform.position.z };
  float32 pAngle = vec2_angle(pos2d, world2d);

  transform.rotation.y = -pAngle + (pi() * 0.5f);

  set_object_transform(ro, state->player, state->playerTransform);

  if(input->fire && state->bullets.size() < 6 && fmod((float32)time, state->fireInterval) < dt) {
    BulletDesc bDesc = {};
    bDesc.startPos = state->playerTransform.position;
    bDesc.direction = vec3_forward(state->playerTransform.rotation);
    bDesc.speed = 0.75f;
    bDesc.lifeTime = 0.4f;
    state->bullets.push_back(spawn_bullet(ro, models, bDesc));
    state->anySelected = true;
  }

  for(uint32 i = 0; i < state->bullets.size(); i++) {
    Bullet& bullet = state->bullets[i];
    if(bullet.valid) {
      BulletResult bResult = update_bullet(ro, &bullet, dt, state->player);
      if(bResult != BulletResult::BULLET_NONE) {
	destroy_bullet(ro, &bullet);
	state->bullets.erase(state->bullets.begin());
	i--;
      }
    }
  }
  
  Vec3 p = state->playerTransform.position;
  state->cameraPos = {p.x, 5.0f, p.z};
  state->cameraRot = { -pi() * 0.5f, 0.0f, 0.0f};
  set_camera_transform(cameras, state->gameCamera, state->cameraPos, state->cameraRot);
  return !input->quit;
}

uint32 editor_mode(GameState* state, GameInput* input, float64 dt, float64 time) {
  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);

  if(!input->mouseLocked && !input->alt)    lock_mouse(true);
  else if(input->mouseLocked && input->alt) lock_mouse(false);

  float32 sensitivity = 0.33f;
  float32 dMouseX = 0.0f;
  float32 dMouseY = 0.0f;
  
  if(input->mouseLocked) {
    dMouseX = (input->rawMousePosition.x - state->lastMousePosition.x) * dt * sensitivity;
    dMouseY = (input->rawMousePosition.y - state->lastMousePosition.y) * dt * sensitivity;
  }

  Vec3 camFront = vec3_normalize(vec3_forward(-state->cameraRot));
  Vec3 camRight = vec3_normalize(vec3_right  (-state->cameraRot));

  state->cameraRot.x = clamp(state->cameraRot.x - dMouseY, pi() * -0.4999f, pi() * 0.4999f);
  state->cameraRot.y -= dMouseX;
  state->cameraSpeed = clamp(input->mouseWheel, -10.0f, 50.0f) + 10.0f;

  Vec3 nextMove = {};
  nextMove.z = input->up    - input->down;
  nextMove.x = input->right - input->left;
  nextMove = vec3_normalize(nextMove);

  Vec3 velocity = (nextMove.z * camFront * dt * state->cameraSpeed) +
                  (nextMove.x * camRight * dt * state->cameraSpeed);

  state->cameraPos.x += velocity.x;
  state->cameraPos.y += velocity.y;
  state->cameraPos.z += velocity.z;

  set_camera_transform(cameras, state->editorCamera, state->cameraPos, state->cameraRot);

  if(input->click) {
    Vec3 world = screen_to_world(cameras, state->editorCamera, input->mousePosition); 
    world = world * 100.0f + state->cameraPos;
    Vec3 outV;
    if(line_cast(ro, state->selected, outV, state->cameraPos, world)) {
      state->anySelected = true;
    }
  }
  //log_("dt: %fms, fps: %f\n", dt, 1.0f/dt); 
  return !input->quit;
}

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

int32 game_update(GameMemory* memory, GameInput* input, float64 dt, float64 time) {
  GameState* state  = (GameState*)memory->memory;
  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);
  
  if(!memory->isInitialized) {
    test_renderer();

    connect_sql(&(state->sqlInfo), pSQLConnectionStr);
   
    memory->offset = sizeof(GameState);
    assert_(memory->offset <= memory->size, 
	    "Too little memory is used! %llu is needed and %llu is available!", 
	    memory->offset, memory->size);
    state->editorCamera = create_camera(cameras);
    state->gameCamera = create_camera(cameras);
    state->light = {};
    state->light.camera = create_camera(cameras);
    state->fireInterval = 1.0f;

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
    res = editor_mode(state, input, dt, time);
    render_loop(get_view_projection(cameras, state->editorCamera), &(state->light));
  }
  else {
    res = game_mode(state, input, dt, time);
    render_loop(get_view_projection(cameras, state->gameCamera), &(state->light));
  }
  update_render_objects(ro, textures);

  if(state->editorMode) {
    render_imgui(memory, state);
  }

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
    disconnect_sql(&(state->sqlInfo));
  }
  return res;
}

void game_quit() {
#if defined(NETWORKING)
  if(isServer) server_shutdown(); 
  if(isClient) client_disconnect();
#endif
}
