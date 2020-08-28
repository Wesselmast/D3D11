#pragma once

struct BulletDesc {
  Vec3 color = {1.0f, 1.0f, 1.0f};
  Vec3 startPos;
  Vec3 direction;
  float32 lifeTime = 0.2f;
  float32 speed = 1.0f;
  bool32 enemyBullet = false;
  uint32 owner;
};

struct Bullet {
  uint32 ref;
  BulletDesc desc;
  bool32 valid = false;
  float32 currentLifeTime = 0.0f;
};

enum BulletResult {
  BULLET_HIT,
  BULLET_LIFETIME,
  BULLET_NONE,
};

Bullet spawn_bullet(RenderObjects* ro, ModelInfo* models, const BulletDesc& desc) {
  uint32 ref = create_model(ro, models, 4);
  Transform bulletT;
  bulletT.position = desc.startPos;
  bulletT.scale = vec3_from_scalar(0.1f);
  set_object_transform(ro, ref, bulletT);
  
  Material material = get_object_material(ro, ref);
  Vec3 red = {1.0f, 0.0f, 0.0f};
  material.materialColor = desc.enemyBullet ? red : desc.color;
  set_object_material(ro, ref, material);

  Bullet bullet = {};
  bullet.ref = ref;
  bullet.desc = desc;
  bullet.valid = true;
  
  return bullet;
}

BulletResult update_bullet(RenderObjects* ro, Bullet* bullet, float32 dt, uint32& hit) {
  Transform t = get_object_transform(ro, bullet->ref);
  t.position = t.position + bullet->desc.direction * bullet->desc.speed;
  set_object_transform(ro, bullet->ref, t);

  bullet->currentLifeTime += dt;

  if(bullet->currentLifeTime >= bullet->desc.lifeTime) {
    return BulletResult::BULLET_LIFETIME;
  }
  uint32 outRef;
  if(check_if_in_any_bounds(ro, outRef, t.position)) {
    if(outRef != bullet->desc.owner && outRef != bullet->ref) {
      hit = outRef;
      return BulletResult::BULLET_HIT;
    }
  }

  return BulletResult::BULLET_NONE;
}

void destroy_bullet(RenderObjects* ro, Bullet* bullet) {
  bullet->valid = false;
  destroy_object(ro, bullet->ref);
}

uint32 create_player(RenderObjects* ro, ModelInfo* models, bool32 enemy) {
  uint32 player = create_model(ro, models, 6);

  ObjectDescriptor* desc = get_object_descriptor(ro, player);
  strcpy(desc->name, "Player");
  desc->textureRef = 0;

  Vec3 eCol = { 1.0f, 0.0f, 0.0f };
  Vec3 fCol = { 0.6f, 0.7f, 0.0f };
  desc->material.materialColor = enemy ? eCol : fCol;

  set_object_descriptor(ro, player, desc);
  return player;
}
