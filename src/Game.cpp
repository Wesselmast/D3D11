#pragma once

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

void render_game_ui(GameMemory* memory, GameState* state) {
}

uint32 game_update(GameState* state, GameInput* input, float64 dt, float64 time) {
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
