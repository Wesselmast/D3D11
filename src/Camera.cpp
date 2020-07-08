#pragma once

struct Cameras {
  Mat4* viewMatrixes = nullptr;
  Mat4* projectionMatrixes = nullptr;
  int32 count = 0;
};


Cameras init_cameras(GameMemory* memory, uint32 limit) {
  Cameras cameras = {};
  cameras.viewMatrixes = (Mat4*)reserve(memory, limit * sizeof(Mat4));
  cameras.projectionMatrixes = (Mat4*)reserve(memory, limit * sizeof(Mat4));
  return cameras;
}

Mat4 get_view_projection(Cameras* cameras, const uint32& camera) {
  bool32 valid = (camera < cameras->count) & (camera >= 0); 
  assert(valid, "Trying to get viewprojection matrix of camera that is null or not possessed!");
  return cameras->projectionMatrixes[camera] * cameras->viewMatrixes[camera];
}

Mat4 calculate_perspective() {
  return mat4_perspective_fov(90.0f, ((float32)windowHeight/(float32)windowWidth), 0.1f, 100.0f);  
}

void set_camera_transform(Cameras* cameras, const uint32& camera, Vec3 position, Vec3 rotation) {
  Mat4 mat = 
    mat4_euler_rotation(rotation) *
    mat4_translation(position);
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
