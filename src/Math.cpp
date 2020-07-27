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

struct Transform {
  Vec3 position;
  Vec3 rotation;
  Vec3 scale;
};

float32 clamp(float32 value, float32 min, float32 max) {
  if(value > max) return max;
  if(value < min) return min;
  return value;
}

float32 pi() {
  return 3.14159265359f;
}

float32 degrees_to_radians() {
  return pi() / 180.0f;
}

Vec3 vec3_from_scalar(float32 scalar) {
  return {scalar, scalar, scalar};
}

Vec3 operator-(const Vec3& a) {
  Vec3 result = {};
  result.x = -a.x;
  result.y = -a.y;
  result.z = -a.z;
  return result;
}

float32 vec3_length_squared(const Vec3& vec) {
  return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

float32 vec3_length(const Vec3& vec) {
  float32 len = vec3_length_squared(vec);
  if(len == 0.0f) return 0.0f;
  return sqrt(len);
}

Vec3 operator*(const Vec3& vec, float32 scalar) {
  Vec3 result = {};
  result.x = vec.x * scalar;
  result.y = vec.y * scalar;
  result.z = vec.z * scalar;
  return result;
}

Vec3 operator*(float32 scalar, const Vec3& vec) {
  Vec3 result = {};
  result.x = vec.x * scalar;
  result.y = vec.y * scalar;
  result.z = vec.z * scalar;
  return result;
}

Vec3 operator+(const Vec3& a, const Vec3& b) {
  Vec3 result = {};
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  return result;
}

Vec3 operator-(const Vec3& a, const Vec3& b) {
  Vec3 result = {};
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  return result;
}

Vec3 vec3_lerp(const Vec3& a, const Vec3& b, float32 t) {
  return b * t + a * (1.0f - t);
}

bool vec3_equal(const Vec3& a, const Vec3& b, float32 bounds) {
  return a.x - bounds <= b.x && a.x + bounds >= b.x &&
         a.y - bounds <= b.y && a.y + bounds >= b.y &&
         a.z - bounds <= b.z && a.z + bounds >= b.z;
}

Vec3 vec3_normalize(const Vec3& vec) {
  Vec3 result = {};
  float32 len = vec3_length(vec);
  if(len == 0.0f) return vec3_from_scalar(0.0f);
  result.x = vec.x / len;
  result.y = vec.y / len;
  result.z = vec.z / len;
  return result;
}

Vec3 vec3_cross(const Vec3& a, const Vec3& b) {
  Vec3 result = {};
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

Vec3 vec3_forward(const Vec3& rot) {
  Vec3 result = {};
  result.x =  sin(rot.y);//cos(rot.x) * cos(rot.y);
  result.y = -tan(rot.x);
  result.z =  cos(rot.y);// * sin(rot.y);
  return result;
}

Vec3 vec3_right(const Vec3& rot) {
  return vec3_cross(vec3_forward(rot), {0.0f, -1.0f, 0.0f});
}

void vec3_print(const Vec3& vec) {
  log_("%f, %f, %f\n", vec.x, vec.y, vec.z);
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
