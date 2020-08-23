#pragma once

void render_editor_ui(GameMemory* memory, GameState* state) {
  RenderObjects* ro = &(state->renderObjects);

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
}

uint32 editor_update(GameState* state, GameInput* input, float64 dt, float64 time) {
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

