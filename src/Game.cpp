#pragma once

void render_game_ui(GameMemory* memory, GameState* state) {
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
      char score[10];
      _ltoa(state->accountInfo.score, score, 10);

      char request[256];
      strcpy(request, "type=register&username=");
      strcat(request, state->accountInfo.username);
      strcat(request, "&password=");
      strcat(request, state->accountInfo.password);
      strcat(request, "&initialScore=");
      strcat(request, score);

      int64 result = 0;
      php_request_int(&(state->php), request, &result);

      if(result) {
	log_("successfully registered!\n");
      }
      else {
	log_("this username is already taken!");
      }
    }

    if(ImGui::Button("Login")) {
      char request[256];
      strcpy(request, "type=login&username=");
      strcat(request, state->accountInfo.username);
      strcat(request, "&password=");
      strcat(request, state->accountInfo.password);

      php_request_int(&(state->php), request, &(state->accountInfo.id));
      if(state->accountInfo.id > 0) {
	char id[10];
	_ltoa(state->accountInfo.id, id, 10);
      
	char scoreRequest[256];
	strcpy(scoreRequest, "type=fetch_score&id=");
	strcat(scoreRequest, id);
	
	php_request_int(&(state->php), scoreRequest, &(state->accountInfo.score));

	log_("successfully logged in! Welcome %llu\n", state->accountInfo.score);
	state->accountInfo.loggedin = true;
      }
      else {
	log_("couldn't log in, combination username and password does not exist!");
      }
    }

    ImGui::End();
    return;
  }

  { //Gamba!
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
    if(ImGui::Button("Spend 2500 bucks to spin the wheel!")) {
      int64& score = state->accountInfo.score;

      if(score - gamble >= 0) {
	score -= gamble;
	
	//what's it gonna be?!
	score += rand() % gamble * 2;
	
	char strid[10];
	_ltoa(state->accountInfo.id, strid, 10);
	char strscore[10];
	_ltoa(score, strscore, 10);

	char request[256];
	strcpy(request, "type=set_score&id=");
	strcat(request, strid);
	strcat(request, "&score=");
	strcat(request, strscore);
	
	php_request_int(&(state->php), request, &(state->accountInfo.score));
      }
      else {
	log_("You don't have enough money to spin! D:\n");
      }
    }

    char strscore[10];
    _ltoa(state->accountInfo.score, strscore, 10);
    ImGui::Text("Score: %s", strscore);

    ImGui::NewLine();
    if(ImGui::Button("Log out")) {
      state->accountInfo = {};
      state->accountInfo.loggedin = false;
    } 

    ImGui::End();
  }
}

void reset_player_position(GameState* state) {
  state->playerTransform = state->startTransform;
}

void game_start(GameState* state) {
  RenderObjects* ro = &(state->renderObjects);

  state->startTransform = {{ 0.0f, -7.2f, 0.0f }, vec3_from_scalar(0.0f)};
  state->playerTransform = state->startTransform; 
  state->playerWalkSpeed = 6.0f;
  state->playerRunSpeed = 10.0f;
  state->fireInterval = 0.1f;
  state->maxBullets = 15;

  set_object_transform(ro, state->player, state->playerTransform);
}

uint32 game_update(GameState* state, GameInput* input, float64 dt, float64 time) {
  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);
  
  if(input->mouseLocked) {
    lock_mouse(false);
    log_("this event fires a lot maybe?");
  }
  state->playerSpeed = input->shift ? state->playerRunSpeed : state->playerWalkSpeed;

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

  if(input->fire && state->bullets.size() < state->maxBullets && fmod((float32)time, state->fireInterval) < dt) {
    BulletDesc bDesc = {};
    bDesc.startPos = state->playerTransform.position;
    bDesc.direction = vec3_forward(state->playerTransform.rotation);
    bDesc.speed = 0.75f;
    bDesc.lifeTime = 1.0f;
    state->bullets.push_back(spawn_bullet(ro, models, bDesc));
    state->anySelected = true;
  }

  for(uint32 i = 0; i < state->bullets.size(); i++) {
    Bullet& bullet = state->bullets[i];
    if(bullet.valid) {
      uint32 outRef;
      BulletResult bResult = update_bullet(ro, &bullet, dt, state->player, outRef);
      if(bResult == BulletResult::BULLET_HIT) {
#if defined(NETWORKING)
	for(uint32 c = 0; c < MAX_NUMBER_OF_CONNECTIONS; c++) {
	  if(otherPlayers[c] == outRef || serverBridge.players[c] == outRef) {
	    state->accountInfo.score += 500;
	  }
	} 
#endif
      }
      
      if(bResult != BulletResult::BULLET_NONE) {
	destroy_bullet(ro, &bullet);
	state->bullets.erase(state->bullets.begin() + i);
	i = 0;
      }
    }
  }

  Vec3 p = state->playerTransform.position;
  state->cameraPos = {p.x, 5.0f, p.z};
  state->cameraRot = { -pi() * 0.5f, 0.0f, 0.0f};
  set_camera_transform(cameras, state->gameCamera, state->cameraPos, state->cameraRot);
  return !input->quit;
}
