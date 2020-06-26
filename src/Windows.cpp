#include <cstdint>
#include <stdio.h>
#include <string.h>

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float  float32;
typedef double float64;

typedef int32_t bool32;

void assert(bool condition, const char* message = "Unspecified termination");
void full_path(char* buffer, const char* fileName);
  
void draw_triangle();

#include "Game.cpp"

#include <windows.h>
//#include "OpenGLRenderer.cpp"
#include <cstdlib>
#include <wchar.h>

#include "D3DRenderer.cpp"

#include <chrono>

typedef BOOL fptr_wglSwapIntervalEXT(int interval);

void assert(bool condition, const char* message) {
  if(!condition) {
    MessageBox(0, message, "Fatal error!", MB_OK | MB_ICONERROR);
    exit(1);
  }
}

void full_path(char* buffer, const char* fileName) {
  char dirPath[512];
  GetCurrentDirectoryA(512, dirPath);
  strcpy(buffer, dirPath);
  strcat(buffer, "\\..\\..\\");
  strcat(buffer, fileName);
  strcat(buffer, "\0");
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
  case WM_SIZE: {
    refresh_viewport(0, 0, LOWORD(lParam), HIWORD(lParam)); 
    return 0;
  }
  case WM_CREATE: {
    init_renderer(hwnd);
    return 0;
  }
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }
  case WM_CLOSE: {
    DestroyWindow(hwnd);
    return 0;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND create_window(uint16 width, uint16 height, const char* name, bool32 fullscreen = false) {
  WNDCLASS windowClass = { };
  
  windowClass.style = CS_OWNDC;
  windowClass.lpfnWndProc = window_proc;
  windowClass.hInstance = GetModuleHandle(0);
  windowClass.lpszClassName = "LittleTestWindow";
  windowClass.hIcon = LoadIcon(NULL, IDI_INFORMATION);
  assert(RegisterClass(&windowClass));
  
  HWND hwnd = CreateWindowEx(0, windowClass.lpszClassName, name, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			     CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, windowClass.hInstance, 0);
  assert(hwnd, "win32: Couldn't create a window!");
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  HDC hdc = GetDC(hwnd);
  int32 pixelFormat = ChoosePixelFormat(hdc, &pfd);
  assert(pixelFormat, "win32: PixelFormat is not valid!");
  assert(SetPixelFormat(hdc, pixelFormat, &pfd), "win32: Couldn't set the PixelFormat!");
  DescribePixelFormat(hdc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  ReleaseDC(hwnd, hdc);
  return hwnd;
}

void close_window(HWND& hwnd, HDC& hdc, HGLRC& hrc) {
  wglMakeCurrent(0, 0);
  wglDeleteContext(hrc);
  ReleaseDC(hwnd, hdc);
} 

int main() {  
  char name[256] = "LittleTest";
#if defined(DEBUG)
  strcat(name, " : DEBUG");
#elif defined(RELEASE)
  strcat(name, " : RELEASE");
#endif

  const uint16 windowWidth = 620; 
  const uint16 windowHeight = 620; 
  
  HWND window = create_window(windowWidth, windowHeight, name, false);
  assert(window, "win32: Couldn't create a window!");

  HDC hdc = GetDC(window);
  HGLRC hrc = wglCreateContext(hdc);
  assert(wglMakeCurrent(hdc, hrc), "Coulnd't make context current");
  
  fptr_wglSwapIntervalEXT* wglSwapInterval = (fptr_wglSwapIntervalEXT*)wglGetProcAddress("wglSwapIntervalEXT");
  if(wglSwapInterval) {
    wglSwapInterval(1);
  }
  
  test_renderer();

  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);


  std::chrono::high_resolution_clock timer;

  MSG message;
  while(1) {
    auto start = timer.now();
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {     //@ToDo: DO THIS BETTER SOMEHOW
      if(message.message == WM_QUIT) goto quit;
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    clear_buffer(1.0f, 0.0f, 1.0f, 1.0f);
    game_update();
    refresh_viewport(0, 0, windowWidth, windowHeight);
    swap_buffers(true);
    float32 dt = std::chrono::duration<float>(timer.now() - start).count();
    printf("dt: %f, fps: %f\n", dt, 1.0f/dt); 
  }
 quit:
  close_window(window, hdc, hrc);
  return 0;
}
