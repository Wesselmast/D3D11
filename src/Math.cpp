#include <math.h> 

struct Mat4 {
  float32 m0[4];
  float32 m1[4];
  float32 m2[4];
  float32 m3[4];
};

struct Mat3 {
  float32 m0[4];
  float32 m1[4];
  float32 m2[4];
};

struct Vec2 {
  float32 x;
  float32 y;
};

struct Vec3 {
  float32 x;
  float32 y;
  float32 z;
};

struct Vec4 {
  float32 x;
  float32 y;
  float32 z;
  float32 w;
};

float32 pi() {
  return acos(-1.0f);
}

float32 degrees_to_radians() {
  return pi() / 180.0f;
}

Vec3 vec3_from_scalar(float32 scalar) {
  return {scalar, scalar, scalar};
}

void mat4_print(const Mat4& mat) {
  log_("%f, %f, %f, %f\n", mat.m0[0], mat.m0[1], mat.m0[2], mat.m0[3]);
  log_("%f, %f, %f, %f\n", mat.m1[0], mat.m1[1], mat.m1[2], mat.m1[3]);
  log_("%f, %f, %f, %f\n", mat.m2[0], mat.m2[1], mat.m2[2], mat.m2[3]);
  log_("%f, %f, %f, %f\n", mat.m3[0], mat.m3[1], mat.m3[2], mat.m3[3]);
}

Mat4 mat4_identity() {
  Mat4 mat = {
    { 1.0f,  0.0f,  0.0f,  0.0f },
    { 0.0f,  1.0f,  0.0f,  0.0f },
    { 0.0f,  0.0f,  1.0f,  0.0f },
    { 0.0f,  0.0f,  0.0f,  1.0f }
  };
  return mat;
}

Mat4 operator*(const Mat4& a, const Mat4& b) {  //@Performance: SIMD
  Mat4 mat = {};

  mat.m0[0] = a.m0[0] * b.m0[0] + a.m0[1] * b.m1[0] + a.m0[2] * b.m2[0] + a.m0[3] * b.m3[0]; 
  mat.m0[1] = a.m0[0] * b.m0[1] + a.m0[1] * b.m1[1] + a.m0[2] * b.m2[1] + a.m0[3] * b.m3[1]; 
  mat.m0[2] = a.m0[0] * b.m0[2] + a.m0[1] * b.m1[2] + a.m0[2] * b.m2[2] + a.m0[3] * b.m3[2]; 
  mat.m0[3] = a.m0[0] * b.m0[3] + a.m0[1] * b.m1[3] + a.m0[2] * b.m2[3] + a.m0[3] * b.m3[3]; 

  mat.m1[0] = a.m1[0] * b.m0[0] + a.m1[1] * b.m1[0] + a.m1[2] * b.m2[0] + a.m1[3] * b.m3[0]; 
  mat.m1[1] = a.m1[0] * b.m0[1] + a.m1[1] * b.m1[1] + a.m1[2] * b.m2[1] + a.m1[3] * b.m3[1]; 
  mat.m1[2] = a.m1[0] * b.m0[2] + a.m1[1] * b.m1[2] + a.m1[2] * b.m2[2] + a.m1[3] * b.m3[2]; 
  mat.m1[3] = a.m1[0] * b.m0[3] + a.m1[1] * b.m1[3] + a.m1[2] * b.m2[3] + a.m1[3] * b.m3[3]; 

  mat.m2[0] = a.m2[0] * b.m0[0] + a.m2[1] * b.m1[0] + a.m2[2] * b.m2[0] + a.m2[3] * b.m3[0]; 
  mat.m2[1] = a.m2[0] * b.m0[1] + a.m2[1] * b.m1[1] + a.m2[2] * b.m2[1] + a.m2[3] * b.m3[1]; 
  mat.m2[2] = a.m2[0] * b.m0[2] + a.m2[1] * b.m1[2] + a.m2[2] * b.m2[2] + a.m2[3] * b.m3[2]; 
  mat.m2[3] = a.m2[0] * b.m0[3] + a.m2[1] * b.m1[3] + a.m2[2] * b.m2[3] + a.m2[3] * b.m3[3]; 

  mat.m3[0] = a.m3[0] * b.m0[0] + a.m3[1] * b.m1[0] + a.m3[2] * b.m2[0] + a.m3[3] * b.m3[0]; 
  mat.m3[1] = a.m3[0] * b.m0[1] + a.m3[1] * b.m1[1] + a.m3[2] * b.m2[1] + a.m3[3] * b.m3[1]; 
  mat.m3[2] = a.m3[0] * b.m0[2] + a.m3[1] * b.m1[2] + a.m3[2] * b.m2[2] + a.m3[3] * b.m3[2]; 
  mat.m3[3] = a.m3[0] * b.m0[3] + a.m3[1] * b.m1[3] + a.m3[2] * b.m2[3] + a.m3[3] * b.m3[3]; 

  return mat;
}

Mat4 operator*(const Mat4& a, float32 b) {
  Mat4 mat = {};
  mat.m0[0] = a.m0[0] * b;
  mat.m0[1] = a.m0[1] * b;
  mat.m0[2] = a.m0[2] * b;
  mat.m0[3] = a.m0[3] * b;
  
  mat.m1[0] = a.m1[0] * b;
  mat.m1[1] = a.m1[1] * b;
  mat.m1[2] = a.m1[2] * b;
  mat.m1[3] = a.m1[3] * b;
  
  mat.m2[0] = a.m2[0] * b;
  mat.m2[1] = a.m2[1] * b;
  mat.m2[2] = a.m2[2] * b;
  mat.m2[3] = a.m2[3] * b;
  
  mat.m3[0] = a.m3[0] * b;
  mat.m3[1] = a.m3[1] * b;
  mat.m3[2] = a.m3[2] * b;
  mat.m3[3] = a.m3[3] * b;
  
  return mat;
}

Mat4 mat4_perspective(float32 width, float32 height, float32 near, float32 far) {
  Mat4 mat = {};

  float32 twoNear = near + near;
  float32 fRange = far / (far - near);
  mat.m0[0] = twoNear / width; 
  mat.m1[1] = twoNear / height;
  mat.m2[2] = fRange;
  mat.m2[3] = 1.0f;
  mat.m3[2] = -fRange * near;

  return mat;
}

Mat4 mat4_perspective_fov(float32 fov, float32 aspect, float32 near, float32 far) {
  float32 cosFov = (float32)cos(0.5f * fov * degrees_to_radians());
  float32 sinFov = (float32)sin(0.5f * fov * degrees_to_radians());

  float32 height = cosFov / sinFov;
  float32 width = height / aspect;
  float32 fRange = far / (far - near);

  Mat4 mat = {};
  mat.m0[0] = width; 
  mat.m1[1] = height;
  mat.m2[2] = fRange;
  mat.m2[3] = 1.0f;
  mat.m3[2] = -fRange * near;
  return mat;
}

Mat4 mat4_transpose(const Mat4& mat) { //@Performance: SIMD
  Mat4 result = {};
  
  result.m0[0] = mat.m0[0];
  result.m0[1] = mat.m1[0];
  result.m0[2] = mat.m2[0]; 
  result.m0[3] = mat.m3[0]; 

  result.m1[0] = mat.m0[1]; 
  result.m1[1] = mat.m1[1];
  result.m1[2] = mat.m2[1]; 
  result.m1[3] = mat.m3[1]; 

  result.m2[0] = mat.m0[2]; 
  result.m2[1] = mat.m1[2]; 
  result.m2[2] = mat.m2[2];
  result.m2[3] = mat.m3[2]; 

  result.m3[0] = mat.m0[3]; 
  result.m3[1] = mat.m1[3]; 
  result.m3[2] = mat.m2[3]; 
  result.m3[3] = mat.m3[3];

  return result;
}

Mat4 mat4_z_rotation(float angle) {
  Mat4 mat = {
     { ((float32)cos(angle)),  ((float32)sin(angle)), 0.0f, 0.0f },
     { ((float32)-sin(angle)), ((float32)cos(angle)), 0.0f, 0.0f },
     { 0.0f,                   0.0f,                  1.0f, 0.0f },
     { 0.0f,                   0.0f,                  0.0f, 1.0f }
  };
  return mat;
}

Mat4 mat4_y_rotation(float angle) {
  Mat4 mat = {
     { ((float32)cos(angle)), 0.0f, ((float32)-sin(angle)), 0.0f },
     { 0.0f,                  1.0f, 0.0f,                   0.0f },
     { ((float32)sin(angle)), 0.0f, ((float32)cos(angle)),  0.0f },
     { 0.0f, 0.0f,                   0.0f,                  1.0f }
  }; 
  return mat;
}

Mat4 mat4_x_rotation(float angle) {
  Mat4 mat = {
     { 1.0f, 0.0f,                   0.0f,                  0.0f },
     { 0.0f, ((float32)cos(angle)),  ((float32)sin(angle)), 0.0f }, 
     { 0.0f, ((float32)-sin(angle)), ((float32)cos(angle)), 0.0f }, 
     { 0.0f, 0.0f,                   0.0f,                  1.0f }
  }; 
  return mat;
}

Mat4 mat4_euler_rotation(const Vec3& rot) {
  Mat4 mat = 
    mat4_z_rotation(rot.z) *
    mat4_y_rotation(rot.y) *
    mat4_x_rotation(rot.x);
  return mat;
}

Mat4 mat4_scaling(const Vec3& v) {
  Mat4 mat = {
     { v.x , 0.0f, 0.0f, 0.0f },
     { 0.0f, v.y , 0.0f, 0.0f },
     { 0.0f, 0.0f, v.z , 0.0f },
     { 0.0f, 0.0f, 0.0f, 1.0f }
  };
  return mat;  
}

Mat4 mat4_translation(const Vec3& v) {
  Mat4 mat = {
     { 1.0f, 0.0f, 0.0f, 0.0f },
     { 0.0f, 1.0f, 0.0f, 0.0f },
     { 0.0f, 0.0f, 1.0f, 0.0f },
     { v.x,  v.y,  v.z,  1.0f}
  };
  return mat;
}

float32 mat3_determinant(const Mat3& mat) {
  float32 result = 
    (mat.m0[0] * mat.m1[1] * mat.m2[2]) +
    (mat.m0[1] * mat.m1[2] * mat.m2[0]) +
    (mat.m0[2] * mat.m1[0] * mat.m2[1]) -

    (mat.m0[2] * mat.m1[1] * mat.m2[0]) -
    (mat.m0[1] * mat.m1[0] * mat.m2[2]) -
    (mat.m0[0] * mat.m1[2] * mat.m2[1]);

  return result;
}

Mat4 mat4_adjugate(const Mat4& mat) {
  Mat4 result = {};

  // M0
  result.m0[0] = mat3_determinant({
      { mat.m1[1], mat.m1[2], mat.m1[3] },
      { mat.m2[1], mat.m2[2], mat.m2[3] },
      { mat.m3[1], mat.m3[2], mat.m3[3] }
  });
  result.m0[1] = mat3_determinant({
      { -mat.m1[0], -mat.m1[2], -mat.m1[3] },
      { -mat.m2[0], -mat.m2[2], -mat.m2[3] },
      { -mat.m3[0], -mat.m3[2], -mat.m3[3] }
  });
  result.m0[2] = mat3_determinant({
      { mat.m1[0], mat.m1[1], mat.m1[3] },
      { mat.m2[0], mat.m2[1], mat.m2[3] },
      { mat.m3[0], mat.m3[1], mat.m3[3] }
  });
  result.m0[3] = mat3_determinant({
      { -mat.m1[0], -mat.m1[1], -mat.m1[2] },
      { -mat.m2[0], -mat.m2[1], -mat.m2[2] },
      { -mat.m3[0], -mat.m3[1], -mat.m3[2] }
  });

  // M1
  result.m1[0] = mat3_determinant({
      { -mat.m0[1], -mat.m0[2], -mat.m0[3] },
      { -mat.m2[1], -mat.m2[2], -mat.m2[3] },
      { -mat.m3[1], -mat.m3[2], -mat.m3[3] }
  });
  result.m1[1] = mat3_determinant({
      { mat.m0[0], mat.m0[2], mat.m0[3] },
      { mat.m2[0], mat.m2[2], mat.m2[3] },
      { mat.m3[0], mat.m3[2], mat.m3[3] }
  });
  result.m1[2] = mat3_determinant({
      { -mat.m0[0], -mat.m0[1], -mat.m0[3] },
      { -mat.m2[0], -mat.m2[1], -mat.m2[3] },
      { -mat.m3[0], -mat.m3[1], -mat.m3[3] }
  });
  result.m1[3] = mat3_determinant({
      { mat.m0[0], mat.m0[1], mat.m0[2] },
      { mat.m2[0], mat.m2[1], mat.m2[2] },
      { mat.m3[0], mat.m3[1], mat.m3[2] }
  });

  // M2
  result.m2[0] = mat3_determinant({
      { mat.m0[1], mat.m0[2], mat.m0[3] },
      { mat.m1[1], mat.m1[2], mat.m1[3] },
      { mat.m3[1], mat.m3[2], mat.m3[3] }
  });
  result.m2[1] = mat3_determinant({
      { -mat.m0[0], -mat.m0[2], -mat.m0[3] },
      { -mat.m1[0], -mat.m1[2], -mat.m1[3] },
      { -mat.m3[0], -mat.m3[2], -mat.m3[3] }
  });
  result.m2[2] = mat3_determinant({
      { mat.m0[0], mat.m0[1], mat.m0[3] },
      { mat.m1[0], mat.m1[1], mat.m1[3] },
      { mat.m3[0], mat.m3[1], mat.m3[3] }
  });
  result.m2[3] = mat3_determinant({
      { -mat.m0[0], -mat.m0[1], -mat.m0[2] },
      { -mat.m1[0], -mat.m1[1], -mat.m1[2] },
      { -mat.m3[0], -mat.m3[1], -mat.m3[2] }
  });

  // M3
  result.m3[0] = mat3_determinant({
      { -mat.m0[1], -mat.m0[2], -mat.m0[3] },
      { -mat.m1[1], -mat.m1[2], -mat.m1[3] },
      { -mat.m2[1], -mat.m2[2], -mat.m2[3] }
  });
  result.m3[1] = mat3_determinant({
      { mat.m0[0], mat.m0[2], mat.m0[3] },
      { mat.m1[0], mat.m1[2], mat.m1[3] },
      { mat.m2[0], mat.m2[2], mat.m2[3] }
  });
  result.m3[2] = mat3_determinant({
      { -mat.m0[0], -mat.m0[1], -mat.m0[3] },
      { -mat.m1[0], -mat.m1[1], -mat.m1[3] },
      { -mat.m2[0], -mat.m2[1], -mat.m2[3] }
  });
  result.m3[3] = mat3_determinant({
      { mat.m0[0], mat.m0[1], mat.m0[2] },
      { mat.m1[0], mat.m1[1], mat.m1[2] },
      { mat.m2[0], mat.m2[1], mat.m2[2] }
  });

  return mat4_transpose(result);
}

float32 mat4_determinant(const Mat4& mat) {
  float32 result =
    (mat.m0[0] * mat.m1[1] * mat.m2[2] * mat.m3[3]) +
    (mat.m0[0] * mat.m1[2] * mat.m2[3] * mat.m3[1]) +
    (mat.m0[0] * mat.m1[3] * mat.m2[1] * mat.m3[2]) -

    (mat.m0[0] * mat.m1[3] * mat.m2[2] * mat.m3[1]) -
    (mat.m0[0] * mat.m1[2] * mat.m2[1] * mat.m3[3]) -
    (mat.m0[0] * mat.m1[1] * mat.m2[3] * mat.m3[2]) -

    (mat.m0[1] * mat.m1[0] * mat.m2[2] * mat.m3[3]) -
    (mat.m0[2] * mat.m1[0] * mat.m2[3] * mat.m3[1]) -
    (mat.m0[3] * mat.m1[0] * mat.m2[1] * mat.m3[2]) +

    (mat.m0[3] * mat.m1[0] * mat.m2[2] * mat.m3[1]) +
    (mat.m0[2] * mat.m1[0] * mat.m2[1] * mat.m3[3]) +
    (mat.m0[1] * mat.m1[0] * mat.m2[3] * mat.m3[2]) +

    (mat.m0[1] * mat.m1[2] * mat.m2[0] * mat.m3[3]) +
    (mat.m0[2] * mat.m1[3] * mat.m2[0] * mat.m3[1]) +
    (mat.m0[3] * mat.m1[1] * mat.m2[0] * mat.m3[2]) -

    (mat.m0[3] * mat.m1[2] * mat.m2[0] * mat.m3[1]) -
    (mat.m0[2] * mat.m1[1] * mat.m2[0] * mat.m3[3]) -
    (mat.m0[1] * mat.m1[3] * mat.m2[0] * mat.m3[2]) -

    (mat.m0[1] * mat.m1[2] * mat.m2[3] * mat.m3[0]) -
    (mat.m0[2] * mat.m1[3] * mat.m2[1] * mat.m3[0]) -
    (mat.m0[3] * mat.m1[1] * mat.m2[2] * mat.m3[0]) +

    (mat.m0[3] * mat.m1[2] * mat.m2[1] * mat.m3[0]) +
    (mat.m0[2] * mat.m1[1] * mat.m2[3] * mat.m3[0]) +
    (mat.m0[1] * mat.m1[3] * mat.m2[2] * mat.m3[0]);
  
  return result;
}

Mat4 mat4_inverse(Mat4 mat) {
  float32 d = 1.0f / mat4_determinant(mat);
  return mat4_adjugate(mat) * d;
}
