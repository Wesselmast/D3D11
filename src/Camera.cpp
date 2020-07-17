#pragma once

const uint32 CAMERA_LIMIT = 1;

struct Cameras {
  Mat4 viewMatrixes[CAMERA_LIMIT];
  Mat4 projectionMatrixes[CAMERA_LIMIT];
  int32 count = 0;
};

Mat4 get_view_projection(Cameras* cameras, const uint32& camera) {
  assert((camera < cameras->count) & (camera >= 0), "Trying to get viewprojection matrix of nonexistent camera!");
  return cameras->viewMatrixes[camera] * cameras->projectionMatrixes[camera];
}

Mat4 calculate_perspective() {
  return mat4_perspective_fov(68.0f, ((float32)windowHeight/(float32)windowWidth), 0.1f, 1000.0f);  
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
