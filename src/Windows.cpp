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

void assert(bool condition, const char* message);

#include "Game.cpp"

#include <windows.h>
//#include "OpenGLRenderer.cpp"
#include "D3DRenderer.cpp"


#include <cstdlib>
#include <wchar.h>

typedef BOOL fptr_wglSwapIntervalEXT(int interval);

void assert(bool condition, const char* message = "Unspecified termination") {
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
  RegisterClass(&windowClass);
  
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
  game_update();
  
  char name[256] = "LittleTest";
#if defined(DEBUG)
  strcat(name, " : DEBUG");
#elif defined(RELEASE)
  strcat(name, " : RELEASE");
#endif

  const uint16 windowWidth = 620; 
  const uint16 windowHeight = 480; 
  
  HWND window = create_window(windowWidth, windowHeight, name, false);
  assert(window, "win32: Couldn't create a window!");

  HDC hdc = GetDC(window);
  HGLRC hrc = wglCreateContext(hdc);
  assert(wglMakeCurrent(hdc, hrc));
  
  fptr_wglSwapIntervalEXT* wglSwapInterval = (fptr_wglSwapIntervalEXT*)wglGetProcAddress("wglSwapIntervalEXT");
  if(wglSwapInterval) {
    wglSwapInterval(1);
  }
  

  // DXD11
  ID3D11Device* device = nullptr;
  IDXGISwapChain* swapchain = nullptr;
  ID3D11DeviceContext* context = nullptr;
  ID3D11RenderTargetView* target = nullptr;

  DXGI_SWAP_CHAIN_DESC swapDesc = {};
  swapDesc.BufferDesc.Width = 0;
  swapDesc.BufferDesc.Height = 0;
  swapDesc.BufferDesc.RefreshRate.Numerator = 0;
  swapDesc.BufferDesc.RefreshRate.Denominator = 0;
  swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; 
  swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  swapDesc.SampleDesc.Count = 1;
  swapDesc.SampleDesc.Quality = 0;
  swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapDesc.BufferCount = 1; //double buffer (1 front 1 back)
  swapDesc.OutputWindow = window;
  swapDesc.Windowed = TRUE;
  swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapDesc.Flags = 0;
 
  D3D11CreateDeviceAndSwapChain(
    nullptr,
    D3D_DRIVER_TYPE_HARDWARE,
    nullptr,
    0,
    nullptr,
    0,
    D3D11_SDK_VERSION,
    &swapDesc,
    &swapchain,
    &device,
    nullptr,
    &context
  );

  
  ID3D11Resource* backBuffer = nullptr;
  swapchain->GetBuffer(0, __uuidof(ID3D11Resource), ((void**)&backBuffer));
  device->CreateRenderTargetView(
    backBuffer,
    nullptr,
    &target
  );
  if(backBuffer) backBuffer->Release();
  
  //

  ID3DBlob* vBlob = nullptr;
  ID3DBlob* fBlob = nullptr;

  char path[512];
  full_path(path, "src\\VertexShader.hlsl");
  wchar_t vPath[strlen(path)+1];
  std::mbstowcs(vPath, path, strlen(path)+1);

  full_path(path, "src\\FragmentShader.hlsl");
  wchar_t fPath[strlen(path)+1];
  std::mbstowcs(fPath, path, strlen(path)+1);

  d3d_assert(compile_vertex_shader(&vBlob, vPath));
  d3d_assert(compile_fragment_shader(&fBlob, fPath));

  D3D11_VIEWPORT viewport;
  viewport.Width = windowWidth;
  viewport.Height = windowHeight;
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  
  MSG message;
  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);
  
   while(1) {
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {     //@ToDo: DO THIS BETTER SOMEHOW
      if(message.message == WM_QUIT) goto quit;
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    clear_buffer(context, target, 1.0f, 0.0f, 1.0f, 1.0f);
    draw_triangle(context, device, target, &viewport, &vBlob, &fBlob);
    //render();
    swap_buffers(swapchain);
    //SwapBuffers(hdc);
  }
 quit:
  close_window(window, hdc, hrc);

  if(context)   context->Release();
  if(target)    target->Release();
  if(swapchain) swapchain->Release();
  if(device)    device->Release();
  return 0;
}
