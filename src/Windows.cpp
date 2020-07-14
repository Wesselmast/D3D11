#include <cstdint>
#include <cstdlib>
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

void pop_error();

#if defined(DEBUG)
#define log_(message, ...) printf(message, ##__VA_ARGS__)
#define assert(condition, message, ...) \
  if(!(condition)) { \
    log_(message, ##__VA_ARGS__); \
    log_("\n"); \
    pop_error(); \
    exit(1); \
  }
#else
#define log_(message, ...)
#define assert(condition, message, ...)
#endif

void full_path(char* buffer, const char* fileName);
  
#include "Math.cpp"
#include "File.cpp"

static const uint16 windowWidth = 620;   //@Note: These dont update. The mark the start size
static const uint16 windowHeight = 480;  //@Note: These dont update. The mark the start size

#include "Memory.h"
#include "Game.cpp"

#include <windows.h>
#include <cstdlib>
#include <wchar.h>
#include <chrono>

typedef BOOL fptr_wglSwapIntervalEXT(int interval);

void pop_error() {
  MessageBox(0, "A fatal error has occured, check the console!", "Fatal error!", MB_OK|MB_ICONERROR);
}

void full_path(char* buffer, const char* fileName) {
  char dirPath[512];
  GetCurrentDirectoryA(512, dirPath);
  strcpy(buffer, dirPath);
  strcat(buffer, "\\");
  strcat(buffer, fileName);
  strcat(buffer, "\0");
}

void lock_mouse(HWND window, bool32 confine) {
  if(confine) {
    RECT rect;
    GetClientRect(window, &rect);
    MapWindowPoints(window, nullptr, (POINT*)(&rect), 2);
    ClipCursor(&rect); 
    ShowCursor(false);
 }
  else {
    ShowCursor(true);
    ClipCursor(nullptr);
  }
} 


static GameInput input;

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
  case WM_SIZE: { 
    refresh_viewport(0, 0, LOWORD(lParam), HIWORD(lParam)); 
    return 0;
  }
  case WM_ACTIVATE: {
    if(wParam & WA_ACTIVE) {
      lock_mouse(hwnd, true);
    }
    else {
      lock_mouse(hwnd, false);
    }
    return 0;
  }
  case WM_LBUTTONDOWN: {
    SetForegroundWindow(hwnd);
    lock_mouse(hwnd, true);
    return 0;
  }
  case WM_CREATE: {
    init_renderer(hwnd);
    return 0;
  }
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP: {
    bool32 wasDown = ((lParam & (1 << 30)) != 0);
    bool32 isDown  = ((lParam & (1 << 31)) == 0);
    if(wasDown == isDown) return 0;
    switch(wParam) {
      case 'W': input.up = isDown;         return 0;
      case 'A': input.left = isDown;       return 0;
      case 'S': input.down = isDown;       return 0;
      case 'D': input.right = isDown;      return 0;
      case VK_ESCAPE: input.quit = isDown; return 0;
    }
    return 0;
  }
    
  case WM_INPUT: {
    uint32 size;
    if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1) {
      log_("input missed 1\n");
      return 0;
    }
    char raw[size];
    if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (void*)(&raw), &size, sizeof(RAWINPUTHEADER)) != size) {    
      log_("input missed 2\n");
      return 0;
    }

    const RAWINPUT& rawInput = (const RAWINPUT&)(raw);
    
    bool32 rawInputFlags = rawInput.header.dwType == RIM_TYPEMOUSE && (rawInput.data.mouse.lLastX != 0 || rawInput.data.mouse.lLastY != 0);
    if(rawInputFlags) {
      input.mousePosition.x += rawInput.data.mouse.lLastX;
      input.mousePosition.y += rawInput.data.mouse.lLastY;
      log_("X: %f, Y: %f\n", input.mousePosition.x, input.mousePosition.y);
    }
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
  windowClass.hIcon = LoadIcon(0, IDI_INFORMATION);
  windowClass.hCursor = LoadCursor(0, IDC_ARROW);
  
  HRESULT registerResult = RegisterClass(&windowClass); 
  assert(registerResult, "win32: Couldn't register window class!");
  
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
  HRESULT pixelFormatResult = SetPixelFormat(hdc, pixelFormat, &pfd); 
  
  assert(pixelFormatResult, "win32: Couldn't set the PixelFormat!");
  DescribePixelFormat(hdc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  ReleaseDC(hwnd, hdc);
  return hwnd;
}

void close_window(HWND& hwnd, HDC& hdc, HGLRC& hrc) {
  wglMakeCurrent(0, 0);
  wglDeleteContext(hrc);
  ReleaseDC(hwnd, hdc);
} 

#if defined(DEBUG)
int main() {
#else
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
#endif
  srand(time(0));

  char name[256] = "LittleTest";
#if defined(DEBUG)
  strcat(name, " : DEBUG");
#elif defined(RELEASE)
  strcat(name, " : RELEASE");
#endif
  
  HWND window = create_window(windowWidth, windowHeight, name, false);
  assert(window, "win32: Couldn't create a window!");

  HDC hdc = GetDC(window);
  HGLRC hrc = wglCreateContext(hdc);
  HRESULT result = wglMakeCurrent(hdc, hrc); 
  assert(result, "Coulnd't make context current");
  
  fptr_wglSwapIntervalEXT* wglSwapInterval = (fptr_wglSwapIntervalEXT*)wglGetProcAddress("wglSwapIntervalEXT");
  if(wglSwapInterval) {
    wglSwapInterval(1);
  }

  RAWINPUTDEVICE rawInputDevice = {};
  rawInputDevice.usUsagePage = 0x01;
  rawInputDevice.usUsage = 0x02;
  rawInputDevice.dwFlags = 0;
  rawInputDevice.hwndTarget = nullptr;
  bool32 checkIfRegistered = RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE)) != FALSE;
  assert(checkIfRegistered, "Couldn't register input device");

  GameMemory gameMemory = {};
  gameMemory.size = megabytes(64);
  gameMemory.memory = VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  
  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);

  std::chrono::high_resolution_clock timer;
  float64 time = 0.0;
  float64 dt = 0.015;
  
  MSG message;
  while(true) {
    auto start = timer.now();
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
      if(message.message == WM_QUIT) goto quit;
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    
    clear_buffer(0.5f, 0.0f, 0.5f, 1.0f);
    if(game_update(&gameMemory, &input, dt, time)) PostQuitMessage(0);
    swap_buffers(true);

    dt = std::chrono::duration<float64>(timer.now() - start).count();
    time += dt;
  }
 quit:
  close_window(window, hdc, hrc);
  return 0;
}
