#pragma once

#if defined(NETWORKING)
#include "Server.cpp"
#include "Client.cpp"

static char targetConnectionIP[128];
static int32 targetConnectionPort;
#endif

void game_start(GameState* state);
void game_end(GameState* state);

uint32 render_game_ui(GameMemory* memory, GameState* state) {
  bool32 quit = 0;
  RenderObjects* ro = &(state->renderObjects);

  if(!state->accountInfo.loggedin) { //Log into account via the server
    const ImVec2 nws = { 375, 175 };
    ImGui::SetNextWindowPos(ImVec2((windowWidth  * 0.5f) - (nws.x * 0.5f), 
				   (windowHeight * 0.5f) - (nws.y * 0.5f)));
    ImGui::SetNextWindowSize(nws);
    
    ImGui::Begin("Account", 0, imgui_static_window_flags());
    ImGui::Text("Welcome to *Untitled*"); 
    ImGui::Text("please log in or register to continue!");
    ImGui::NewLine();

    ImGui::Text("your username may only contain letters and numbers");
    ImGui::InputText("* Username", (char*)&(state->accountInfo.username), 128, 
		     ImGuiInputTextFlags_CallbackCharFilter, filter_username);
    ImGui::InputText("* Password", (char*)&(state->accountInfo.password), 128, 
		     ImGuiInputTextFlags_Password);

    if(ImGui::Button("Register")) {
      char score[10];
      _ltoa(state->accountInfo.score, score, 10);

      if(strlen(state->accountInfo.username) < 1) {
	ImGui::OpenPopup("UsernameShortPopup");
	ImGui::End();
	return quit;
      }
      if(strlen(state->accountInfo.password) < 1) {
	ImGui::OpenPopup("PasswordShortPopup");
	ImGui::End();
	return quit;
      }
      
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
	ImGui::OpenPopup("SuccessfulRegPopup");
      }
      else {
	ImGui::OpenPopup("UsernameTakenPopup");
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

	log_("successfully logged in! Welcome %lld\n", state->accountInfo.score);
	state->accountInfo.loggedin = true;
	state->showMainMenu = true;
      }
      else {
	ImGui::OpenPopup("LoginFailedPopup");
      }
    }

    ImVec2 middle(windowWidth  * 0.5f, windowHeight * 0.5f);
    imgui_popup_window("UsernameTakenPopup", middle, 
		       "This username is already taken!");
    imgui_popup_window("PasswordShortPopup", middle, 
		       "Your password is too short!");
    imgui_popup_window("UsernameShortPopup", middle, 
		       "Your username is too short!");
    imgui_popup_window("SuccessfulRegPopup", middle, 
		       "Successfully registered!");
    imgui_popup_window("LoginFailedPopup",   middle, 
		       "couldn't log in, combination username and password does not exist!");
    ImGui::End();
  }

  if(state->showMainMenu) { // Main Menu!
    { //Middle of the screen menu
      const ImVec2 nws = { 215, 150 };
      const ImVec2 bts = { 200, 30  };
      ImGui::SetNextWindowPos(ImVec2((windowWidth  * 0.5f) - (nws.x * 0.5f), 
				     (windowHeight * 0.5f) - (nws.y * 0.5f)));
      ImGui::SetNextWindowSize(nws);
      
      ImGui::Begin("Main Menu", 0, imgui_static_window_flags());

      state->practiceMode = ImGui::Button("PRACTICE", bts);
      ImGui::NewLine();
      state->showNetworking = ImGui::Button("PLAY ONLINE", bts);
      ImGui::NewLine();
      quit = ImGui::Button("QUIT GAME", bts);
      
      state->showMainMenu &= !state->showNetworking;
      state->showMainMenu &= !state->practiceMode;
      state->startGame |= state->practiceMode;

      ImGui::End();
    }

    { //Greeter in the corner!
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(175, 50));
      ImGui::Begin("Corner Greetings", 0, imgui_static_window_flags());
      
      ImGui::Text("Welcome, %s", state->accountInfo.username);
      ImGui::Text("You have %lld bucks!", state->accountInfo.score);
      
      ImGui::End();
    }
  }  

  if(state->showPauseMenu) { // Pause Menu!
    { //Middle of the screen menu
      const ImVec2 nws = { 215, 150 };
      const ImVec2 bts = { 200, 30  };
      ImGui::SetNextWindowPos(ImVec2((windowWidth  * 0.5f) - (nws.x * 0.5f), 
				     (windowHeight * 0.5f) - (nws.y * 0.5f)));
      ImGui::SetNextWindowSize(nws);
      
      ImGui::Begin("Pause Menu", 0, imgui_static_window_flags());
      
      state->showMainMenu = ImGui::Button("MAIN MENU", bts);
      ImGui::NewLine();
      quit = ImGui::Button("QUIT GAME", bts);
      
      state->showPauseMenu = !state->showMainMenu;
      if(state->showMainMenu) {
	game_end(state);
      }

      ImGui::End();
    }

    { //Greeter in the corner!
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImVec2(175, 50));
      ImGui::Begin("Corner Greetings", 0, imgui_static_window_flags());
      
      ImGui::Text("Welcome, %s", state->accountInfo.username);
      ImGui::Text("You have %lld bucks!", state->accountInfo.score);
      
      ImGui::End();
    }
  }

#if defined(NETWORKING)
  if(state->showNetworking) { //Connect to others via IP
    ImGui::Begin("Networking", 0, imgui_default_window_flags());
    
    const uint32 minimalInvestment = 1000; 

    ImGui::InputText("ip", targetConnectionIP, 128);
    ImGui::SameLine();
    ImGui::InputInt("port", &targetConnectionPort);
    
    if(ImGui::Button("Start server")) {
      if(state->server.valid) server_shutdown(&state->server, ro);
      if(state->client.valid) client_disconnect(&state->client, ro);

      if(state->accountInfo.score < minimalInvestment) {
	ImGui::OpenPopup("NotEnoughMoneyPopup");
      }
      else {
	server_startup(&state->server, create_ip_endpoint(targetConnectionIP, (uint16)targetConnectionPort));
	state->accountInfo.score -= state->investment;
	state->showNetworking = false;
	state->startGame = true;
      }
    }
    if(ImGui::Button("Start client")) {
      if(state->server.valid) server_shutdown(&state->server, ro);
      if(state->client.valid) client_disconnect(&state->client, ro);

      if(state->accountInfo.score < minimalInvestment) {
	ImGui::OpenPopup("NotEnoughMoneyPopup");
      }
      else {
	client_connect(&state->client, create_ip_endpoint(targetConnectionIP, (uint16)targetConnectionPort));
	state->accountInfo.score -= state->investment;
	state->showNetworking = false;
	state->startGame = true;
      }
    }  

    ImGui::NewLine();
    ImGui::Text("Invest bucks! You have %lld", state->accountInfo.score);
    ImGui::SliderInt("bucks", &(state->investment), 1000, state->accountInfo.score);
    
    ImVec2 middle(windowWidth  * 0.5f, windowHeight * 0.5f);
    imgui_popup_window("NotEnoughMoneyPopup", middle, 
		       "Unfortunately you don't have enough funds to play anymore.. go home!");
    ImGui::End();
  }
#endif

  if(state->gameOver) {
    { //Middle of the screen menu
      const ImVec2 nws = { 215, 200 };
      const ImVec2 bts = { 200, 30  };
      ImGui::SetNextWindowPos(ImVec2((windowWidth  * 0.5f) - (nws.x * 0.5f), 
				     (windowHeight * 0.5f) - (nws.y * 0.5f)));
      ImGui::SetNextWindowSize(nws);
      
      ImGui::Begin("Game Over", 0, imgui_static_window_flags());


      if(state->gameResult != GameResult::NONE) {
	if(state->gameResult == GameResult::GAME_WON) {
	  state->accountInfo.score += state->investment * 2;
	}
	
	log_("pizzaslices");

	char id[10];
	_ltoa(state->accountInfo.id, id, 10);
	char score[10];
	_ltoa(state->accountInfo.score, score, 10);
	  
	char scoreRequest[256];
	strcpy(scoreRequest, "type=set_score&id=");
	strcat(scoreRequest, id);
	strcat(scoreRequest, "&score=");
	strcat(scoreRequest, score);
	
	int64 result;
	php_request_int(&(state->php), scoreRequest, &result);
	
	state->gameResult = GameResult::NONE;
      }
      
      ImGui::Text("HIGHSCORES:");
      ImGui::NewLine();
      const uint32 amt = 10;
      char data[amt][256];
      parse_top_amount(&state->php, data, amt);
      uint32 offset = 1;
      for(uint32 i = 0; i < amt; i += 2) {
	ImGui::Text("#%d", i + offset);
	ImGui::SameLine();
	ImGui::Text("%s", data[i]);
	ImGui::SameLine(nws.x - 40);
	ImGui::Text("%s", data[i + 1]);
	if(i != amt - 2) {
	  ImGui::Separator();
	}
	offset--;
      }
      ImGui::NewLine();
      state->showMainMenu = ImGui::Button("MAIN MENU", bts);
      if(state->showMainMenu) {
	game_end(state);
      }

      ImGui::End();
    }
  }

  if(!state->playGame && state->startGame) {
    state->showMainMenu = false;
    game_start(state);
  }

  if(state->playGame) {
    { //Hitpoints!
      ImGui::SetNextWindowPos(ImVec2(0, windowHeight - 100));
      ImGui::SetNextWindowSize(ImVec2(200, 50));
      ImGui::Begin("HitpointBar", 0, imgui_static_noinput_window_flags());
      
      ImGui::Text("Hitpoints of %s", state->accountInfo.username);
      ImGui::SliderInt("Hitpoints", &(state->hitpoints), 0, 10);
      
      ImGui::End();
    }
    
  }
  return quit;
}

void game_start(GameState* state) {
  state->playGame = true;

  RenderObjects* ro = &(state->renderObjects);

  state->startTransform = {{ 0.0f, -7.2f, 0.0f }, vec3_from_scalar(0.0f)};
  state->playerTransform = state->startTransform; 
  state->playerWalkSpeed = 6.0f;
  state->playerRunSpeed = 10.0f;
  state->fireInterval = 0.1f;
  state->maxBullets = 15;
  state->hitpoints = 10;

  set_object_transform(ro, state->player, state->playerTransform);
}

void game_end(GameState* state) {
  state->playGame = false;
  state->startGame = false;
  state->practiceMode = false;
  state->gameOver = false;

  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);

  state->playerTransform = state->startTransform; 
  set_object_transform(ro, state->player, state->playerTransform);
  
  set_camera_transform(cameras, state->gameCamera, vec3_from_scalar(0.0f), vec3_from_scalar(0.0f));

#if defined(NETWORKING)
  if(state->server.valid) server_shutdown(&state->server, ro); 
  if(state->client.valid) client_disconnect(&state->client, ro);
#endif
}

uint32 game_update(GameState* state, GameInput* input, float64 dt, float64 time) {
  if(input->mouseLocked) lock_mouse(false);

  if(!state->playGame) return 0;
  if(state->gameOver) return 0;

#if defined(NETWORKING)
  if(state->server.valid) {
    state->networkMode = true;
    server_update(state);
  }
  else if(state->client.valid) {
    state->networkMode = true;
    client_update(state);
  }
  else if(!state->practiceMode) {
    state->networkMode = false;
    state->gameOver = true;
    return 0;
  }
#endif

  RenderObjects* ro = &(state->renderObjects);
  Cameras* cameras  = &(state->cameras);

  state->showPauseMenu ^= input->pauseMenu;
  input->pauseMenu = 0;

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

  if(state->hitpoints <= 0) {
    state->networkMode = false;
    state->gameOver = true;

    log_("AOIJDNAWONDWAJNDWA0");

#if defined(NETWORKING)
    if(state->client.valid) client_player_died(&(state->client), ro);
    if(state->server.valid) server_player_died(&(state->server), ro);
#endif
    state->gameResult = GameResult::GAME_LOST;
    return 0;
  } 

  if(input->fire && state->bullets.size() < state->maxBullets && fmod((float32)time, state->fireInterval) < dt) {
    BulletDesc bDesc = {};
    bDesc.startPos = state->playerTransform.position;
    bDesc.direction = vec3_forward(state->playerTransform.rotation);
    bDesc.speed = 0.75f;
    bDesc.lifeTime = 0.4f;
    bDesc.owner = state->player;

    state->bullets.push_back(spawn_bullet(ro, models, bDesc));
#if defined(NETWORKING)
    if(state->client.valid) client_send_bullet(&(state->client), ro, bDesc);
    if(state->server.valid) server_send_bullet(&(state->server), ro, bDesc);
#endif

    state->anySelected = true;
  }

  for(uint32 i = 0; i < state->bullets.size(); i++) {
    Bullet& bullet = state->bullets[i];
    if(bullet.valid) {
#if defined(NETWORKING)
      uint32 outRef;
      BulletResult bResult = update_bullet(ro, &bullet, dt, outRef);
      if(bResult == BulletResult::BULLET_HIT) {
	if(bullet.desc.enemyBullet) {
	  if(state->player == outRef) {
	    state->hitpoints--;
	  }
	}
	else {
	  for(uint32 c = 0; c < MAX_NUMBER_OF_CONNECTIONS; c++) {
	    if(state->client.otherPlayers[c] == outRef || 
	       state->server.players[c]      == outRef) {
	      log_("got em chief!");
	    }
	  } 
	}
      }
      
      if(bResult != BulletResult::BULLET_NONE) {
	destroy_bullet(ro, &bullet);
	state->bullets.erase(state->bullets.begin() + i);
	i = 0;
      }
    }
#endif
  }

  Vec3 p = state->playerTransform.position;
  state->cameraPos = {p.x, 5.0f, p.z};
  state->cameraRot = { -pi() * 0.5f, 0.0f, 0.0f};
  set_camera_transform(cameras, state->gameCamera, state->cameraPos, state->cameraRot);
  return 0;
}
