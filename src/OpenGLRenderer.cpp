#include "GL/gl.h"

void draw_triangle() {
  float32 r = 1.0f;
  float32 g = 0.25f;
  float32 b = 0.5f;
  glClearColor(r, g, b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glRotatef(0.01f, 0.0f, 0.0f, 1.0f);
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f - r, 1.0f - g, 1.0f - b);
  glVertex2f( 0.0f,  0.5f);
  glVertex2f(-0.5f, -0.5f);
  glVertex2f( 0.5f, -0.5f);
  glEnd();
  glFlush();
}

void refresh_viewport(int32 x, int32 y, uint32 w, uint32 h) {
  glViewport(x, y, w, h); 
}


// void draw() {
//   float32 vertices[] = {
//      0.0f,  0.5f,  
//     -0.5f, -0.5f,
//      0.5f, -0.5f
//   };

//   uint32 indices[] = {
//     0, 1, 2 
//   };

//   GLuint VB;
//   glGenBuffers(1, &VB);
//   glBindBuffer(GL_ARRAY_BUFFER, VB);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//    IB;
//   glGenBuffers(1, &IB);
//   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
//   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


//   // glClearColor(r, g, b, 1.0f);
//   // glClear(GL_COLOR_BUFFER_BIT);
//   glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
  
//   // uint32 VA;
//   // glGenVertexArrays(1, &VA
//   // glBindVertexArray(rendererID);

  
// }
