#pragma once

struct BulletDesc {
  Vec3 startPos;
  Vec3 direction;
  float32 lifeTime = 0.2f;
  float32 speed = 1.0f;
};

struct Bullet {
  uint32 ref;
  BulletDesc desc;
  bool valid = false;
  float32 currentLifeTime = 0.0f;
};

enum BulletResult {
  END_HIT,
  END_LIFETIME,
  NONE,
};

Bullet spawn_bullet(RenderObjects* ro, ModelInfo* models, BulletDesc desc) {
  uint32 bullet = create_model(ro, models, 4);
  Transform bulletT;
  bulletT.position = desc.startPos;
  bulletT.scale = vec3_from_scalar(0.2f);
  set_object_transform(ro, bullet, bulletT);
  return { bullet, desc, true };
}

BulletResult update_bullet(RenderObjects* ro, Bullet* bullet, float32 dt) {
  Transform t = get_object_transform(ro, bullet->ref);
  t.position = t.position + bullet->desc.direction * bullet->desc.speed;
  set_object_transform(ro, bullet->ref, t);

  bullet->currentLifeTime += dt;

  if(bullet->currentLifeTime >= bullet->desc.lifeTime) {
    return BulletResult::END_LIFETIME;
  }
  return BulletResult::NONE;
}

void destroy_bullet(RenderObjects* ro, Bullet* bullet) {
  bullet->valid = false;
  destroy_object(ro, bullet->ref);
}

uint32 create_player(RenderObjects* ro, ModelInfo* models, bool32 enemy) {
  uint32 player = create_model(ro, models, 4);

  ObjectDescriptor* desc = get_object_descriptor(ro, player);
  strcpy(desc->name, "Player");
  desc->textureRef = 0;

  Vec3 eCol = { 1.0f, 0.0f, 0.0f };
  Vec3 fCol = { 1.0f, 1.0f, 0.0f };
  desc->material.materialColor = enemy ? eCol : fCol;

  set_object_descriptor(ro, player, desc);
  return player;
}
