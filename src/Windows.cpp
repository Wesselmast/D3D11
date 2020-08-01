#include "Windows.h"
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

static GameInput input;
static HWND activeWindow;

void lock_mouse(bool32 confine) {
  if(confine) {
    RECT rect;
    GetClientRect(activeWindow, &rect);
    MapWindowPoints(activeWindow, nullptr, (POINT*)(&rect), 2);
    ClipCursor(&rect); 
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
  }
  else {
    ClipCursor(nullptr);
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
  }
  ShowCursor(!confine);
  input.mouseLocked = confine; 
} 

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) return 1;
  
  switch(uMsg) {
  case WM_SIZE: { 
    windowWidth  = LOWORD(lParam);
    windowHeight = HIWORD(lParam);
    return 0;
  }
  case WM_ACTIVATE: {
    activeWindow = hwnd;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    
    ImGui_ImplWin32_Init(hwnd);
    init_ImGUI();
    
    lock_mouse(wParam & WA_ACTIVE);
    return 0;
  }
  case WM_CREATE: {
    init_renderer(hwnd);
    return 0;
  }
  case WM_MOUSEMOVE: {
    input.mousePosition.x = LOWORD(lParam);    
    input.mousePosition.y = HIWORD(lParam);
    return 0;
  }
  case WM_MOUSEWHEEL: {
    input.mouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam)/(float)WHEEL_DELTA;
    return 0;
  }
  case WM_LBUTTONDOWN: {
    SetForegroundWindow(hwnd);
    activeWindow = hwnd;
    if(!imgui_hovering_anything()) {
      input.fire = true; 
    }
    return 0;
  }
  case WM_LBUTTONUP: {
    if(!imgui_hovering_anything()) {
      input.fire = false; 
    }
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
      case 'W': input.up = isDown;          return 0;
      case 'A': input.left = isDown;        return 0;
      case 'S': input.down = isDown;        return 0;
      case 'D': input.right = isDown;       return 0;
      case 'H': input.editorMode = isDown;  return 0;
      case VK_ESCAPE:  input.quit = isDown; return 0;
      case VK_MENU:    input.alt  = isDown; return 0;
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
      input.rawMousePosition.x += rawInput.data.mouse.lLastX;
      input.rawMousePosition.y += rawInput.data.mouse.lLastY;
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
  assert_(registerResult, "win32: Couldn't register window class!");
  
  HWND hwnd = CreateWindowEx(0, windowClass.lpszClassName, name, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			     CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, windowClass.hInstance, 0);
  assert_(hwnd, "win32: Couldn't create a window!");

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  HDC hdc = GetDC(hwnd);
  int32 pixelFormat = ChoosePixelFormat(hdc, &pfd);
  assert_(pixelFormat, "win32: PixelFormat is not valid!");
  HRESULT pixelFormatResult = SetPixelFormat(hdc, pixelFormat, &pfd); 
  
  assert_(pixelFormatResult, "win32: Couldn't set the PixelFormat!");
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
  assert_(window, "win32: Couldn't create a window!");

  HDC hdc = GetDC(window);
  HGLRC hrc = wglCreateContext(hdc);
  HRESULT result = wglMakeCurrent(hdc, hrc); 
  assert_(result, "Coulnd't make context current");
  
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
  assert_(checkIfRegistered, "Couldn't register input device");

  GameMemory gameMemory = {};
  gameMemory.size = megabytes(128);
  gameMemory.memory = (uint8*)VirtualAlloc(0, gameMemory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  
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
    
    if(game_update(&gameMemory, &input, dt, time)) PostQuitMessage(0);
    swap_buffers(true);
    clear_buffer(0.5f, 0.0f, 0.5f, 1.0f);

    dt = std::chrono::duration<float64>(timer.now() - start).count();
    time += dt;
  }
 quit:
  close_window(window, hdc, hrc);
  return 0;
}
