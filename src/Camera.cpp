#pragma once

const uint32 CAMERA_LIMIT = 3;

struct Cameras {
  Mat4 viewMatrixes[CAMERA_LIMIT];
  Mat4 projectionMatrixes[CAMERA_LIMIT];
  int32 count = 0;
};

Mat4 get_view_projection(Cameras* cameras, const uint32& camera) {
  assert_((camera < cameras->count) & (camera >= 0), "Trying to get viewprojection matrix of nonexistent camera!");
  return cameras->viewMatrixes[camera] * cameras->projectionMatrixes[camera];
}

Mat4 calculate_perspective() {
  return mat4_perspective_fov(68.0f, ASPECT_RATIO, 0.1f, 1000.0f);  
}

void set_camera_transform(Cameras* cameras, const uint32& camera, Vec3 position, Vec3 rotation) {
  Mat4 mat =
    mat4_translation(-position) *     
    mat4_euler_rotation(rotation);
  cameras->viewMatrixes[camera] = mat;
  cameras->projectionMatrixes[camera] = calculate_perspective();
}

uint32 create_camera(Cameras* cameras) {
  uint32 index = cameras->count;
  cameras->projectionMatrixes[index] = calculate_perspective();
  cameras->viewMatrixes[index] = mat4_identity();
  cameras->count++;
  return index;
}

// Vec3 world_to_screen(Cameras* cameras, const uint32& camera, const Vec3& vec) {
//   Vec4 vec4D = { vec.x, vec.y, vec.z, 1.0f };
//   Vec4 clipSpace = cameras->projectionMatrixes[camera] *
//                    (cameras->viewMatrixes[camera] * vec4D);
//   Vec3 ndcSpace;
//   if(clipSpace.w == 0.0f) {
//     ndcSpace = vec3_from_scalar(0.0f);
//   }
//   else {
//     ndcSpace = { clipSpace.x / clipSpace.w,
// 		 clipSpace.y / clipSpace.w,
// 		 clipSpace.z / clipSpace.w };
//   }
  
//   log_("NDC SPACE: \n");
//   vec3_print(ndcSpace);
//   return ndcSpace;
// }

Vec3 screen_to_world(Cameras* cameras, const uint32& camera, const Vec2& vec) {
  float32 x = 2.0f * vec.x / windowWidth  - 1.0f;
  float32 y = 2.0f * vec.y / windowHeight - 1.0f;

  Vec4 vec4D = { x, -y, 1.0f, 1.0f };
  Mat4 ivp = mat4_inverse(get_view_projection(cameras, camera));
  Vec4 dir =  ivp * vec4D;
  
  Vec3 dir3D = { dir.x, dir.y, dir.z };
  return dir3D;
}
