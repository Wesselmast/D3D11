bool check_if_in_bounds(RenderObjects* renderObjects, uint32 index, Vec3 point) {
  ObjectDescriptor* descPtr = get_object_descriptor(renderObjects, index); 
  
  Vec3 min = descPtr->transform.position - descPtr->transform.scale; 
  Vec3 max = descPtr->transform.position + descPtr->transform.scale; 

  return point.x < max.x && point.x > min.x &&
         point.y < max.y && point.y > min.y &&
         point.z < max.z && point.z > min.z;
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
