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

  log_("%s\n", descPtr->name);
  log_("xcalc: %f\n", px2);
  log_("xtarg: %f\n", xLen);
  log_("ycalc: %f\n", py2);
  log_("ytarg: %f\n", yLen);
  log_("zcalc: %f\n", pz2);
  log_("ztarg: %f\n", zLen);
  log_("\n");

  
  return px2 <= xLen && 
         py2 <= yLen && 
         pz2 <= zLen;

  // Vec3 i = v2    - v1;
  // Vec3 j = v4    - v1;
  // Vec3 k = v5    - v1;
  // Vec3 v = point - v1;
  
  // float32 iv = vec3_dot(v, i);
  // float32 jv = vec3_dot(v, j);
  // float32 kv = vec3_dot(v, k);

  // float32 i2 = vec3_dot(i, i);
  // float32 j2 = vec3_dot(j, j);
  // float32 k2 = vec3_dot(k, k);
  
  // return iv > 0.0f && iv < i2 &&
  // 	 jv > 0.0f && jv < j2 &&
  // 	 kv > 0.0f && kv < k2;

//   Mat4 vMatMin = mat4_translation(min) * mat4_euler_rotation(descPtr->transform.rotation);
//   Mat4 vMatMax = mat4_translation(max) * mat4_euler_rotation(descPtr->transform.rotation);
//   Vec3 newMin = {vMatMin.m3[0], vMatMin.m3[1], vMatMin.m3[2]};
//   Vec3 newMax = {vMatMax.m3[0], vMatMax.m3[1], vMatMax.m3[2]};

//   log_("MIN %s : \n", descPtr->name);
//   vec3_print(newMin);
//   log_("\n");
//   log_("MAX %s : \n", descPtr->name);
//   vec3_print(newMax);
//   log_("\n");

// // Mat4 mat = mat4_euler_rotation(descPtr->
//   // Vec3 rotMin = 

//   return point.x < newMax.x && point.x > newMin.x &&
//          point.y < newMax.y && point.y > newMin.y &&
//          point.z < newMax.z && point.z > newMin.z;
}  

bool line_cast(RenderObjects* renderObjects, uint32& out, Vec3 start, Vec3 end) { //can def be faster
  Vec3 current = start;
  float32 inc = 0.0f;
  while(!vec3_equal(current, end, 1.0f)) {
    current = vec3_lerp(start, end, inc);

    for(uint32 i = 0; i < renderObjects->count; i++) {
      if(!is_object_valid(renderObjects, i)) continue;
      if(check_if_in_bounds(renderObjects, i, current)) {
	out = i;
	return true;
      }
    }
    inc += 0.02f;
  }
  return false;
}
