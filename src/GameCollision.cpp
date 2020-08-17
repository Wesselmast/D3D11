Vec3 rotate_vertex(const Vec3& vertex, const Vec3& euler) {
  Mat4 mat = mat4_translation(vertex) * mat4_euler_rotation(euler);
  return {mat.m3[0], mat.m3[1], mat.m3[2]};
}

bool check_if_in_bounds(RenderObjects* renderObjects, uint32 index, Vec3 point) {
  ObjectDescriptor* descPtr = get_object_descriptor(renderObjects, index); 
  
  Vec3 s = descPtr->transform.scale;
  Vec3 r = descPtr->transform.rotation;
  Vec3 p = descPtr->transform.position;

  Vec3 o1 = {-s.x, -s.y, -s.z};
  Vec3 o2 = {-s.x, -s.y,  s.z};
  Vec3 o4 = { s.x, -s.y, -s.z};
  Vec3 o5 = {-s.x,  s.y, -s.z};

  Vec3 v1 = rotate_vertex(p + o1, r);
  Vec3 v2 = rotate_vertex(p + o2, r);
  Vec3 v4 = rotate_vertex(p + o4, r);
  Vec3 v5 = rotate_vertex(p + o5, r);

  float32 xLen =  vec3_length_squared(v4 - v1);
  float32 yLen =  vec3_length_squared(v5 - v1);
  float32 zLen =  vec3_length_squared(v2 - v1);
  Vec3 x = v4 - v1 / xLen;
  Vec3 y = v5 - v1 / yLen;
  Vec3 z = v2 - v1 / zLen;
  
  Vec3 dir = point - p;
  float32 px2 = abs(vec3_dot(dir, x)); 
  float32 py2 = abs(vec3_dot(dir, y)); 
  float32 pz2 = abs(vec3_dot(dir, z)); 
  
  return px2 <= xLen && 
         py2 <= yLen && 
         pz2 <= zLen;
}

bool check_if_in_any_bounds(RenderObjects* renderObjects, uint32& outRef, Vec3 point) {
  for(uint32 i = 0; i < renderObjects->count; i++) {
    if(!is_object_valid(renderObjects, i)) continue;
    if(check_if_in_bounds(renderObjects, i, point)) {
      outRef = i;
      return true;
    }
  }
  return false;
}

bool line_cast(RenderObjects* renderObjects, uint32& out, Vec3& outV, Vec3 start, Vec3 end) { //can def be faster
  Vec3 current = start;
  float32 inc = 0.0f;
  while(!vec3_equal(current, end, 1.0f)) {
    current = vec3_lerp(start, end, inc);

    if(check_if_in_any_bounds(renderObjects, out, current)) {
      outV = current;
      return true;
    }
    inc += 0.02f;
  }
  return false;
}
