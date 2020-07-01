#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

struct RenderInfo {
  ID3D11DeviceContext* context = nullptr;
  IDXGISwapChain* swapchain = nullptr;
  ID3D11Device* device = nullptr;
  ID3D11RenderTargetView* target = nullptr;
  ID3D11DepthStencilView* dsv = nullptr;
  D3D11_VIEWPORT* viewport = nullptr;
  ID3DBlob* Vblob = nullptr;
  ID3DBlob* Fblob = nullptr;
};

struct RenderObjects {
  ID3D11Buffer** vertexBuffers = nullptr;
  uint32* vertexBufferStrides = nullptr;
  ID3D11Buffer** indexBuffers = nullptr;
  uint32* indexBufferSizes = nullptr;
  ID3D11Buffer** faceColorBuffers = nullptr;
  ID3D11VertexShader** vertexShaders = nullptr;
  ID3D11PixelShader** fragmentShaders = nullptr;
  ID3D11InputLayout** inputLayouts = nullptr;
  uint32 count = 0;
};

static RenderInfo renderInfo; // @CleanUp: Maybe have this not be a global variable
static RenderObjects renderObjects;

static void d3d_assert(HRESULT err) {
  if(FAILED(err)) {
    printf("D3D ERROR: %d\n", (uint32)err);
    assert(false, "D3D ERROR, check log");
  }
}

static HRESULT compile_shader(LPCWSTR path, LPCSTR target, ID3DBlob** blob) {
  ID3DBlob* errorBlob;

  HRESULT result = D3DCompileFromFile(path, 0, 0, "main", target, 0, 0, blob, &errorBlob );
  if(FAILED(result)) {
    if(errorBlob) {
      printf("%s", (char*)errorBlob->GetBufferPointer());
      errorBlob->Release();
    }
    if(*blob) (*blob)->Release();
    return result;
  }

  return result;  
}

static HRESULT compile_vertex_shader(ID3DBlob** blob, LPCWSTR path) {
  return compile_shader(path, "vs_4_0", blob);
}

static HRESULT compile_fragment_shader(ID3DBlob** blob, LPCWSTR path) {
  return compile_shader(path, "ps_4_0", blob);
}

void test_renderer() {
  assert(renderInfo.device, "device has not been initialized!");
  assert(renderInfo.swapchain, "swapchain has not been initialized!");
  assert(renderInfo.context, "context has not been initialized!");
  assert(renderInfo.target, "rendertarget has not been initialized!");
  assert(renderInfo.Vblob, "vblob has not been initialized!");
  assert(renderInfo.Fblob, "fblob has not been initialized!");
  assert(renderInfo.viewport, "viewport has not been initialized!");
}

void init_renderer(HWND window) {
  ID3D11Device* device = nullptr;
  IDXGISwapChain* swapchain = nullptr;
  ID3D11DeviceContext* context = nullptr;
  ID3D11RenderTargetView* target = nullptr;
  ID3D11DepthStencilView* dsv = nullptr;
  ID3DBlob* Vblob = nullptr;
  ID3DBlob* Fblob = nullptr;

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

  assert(swapchain, "Swapchain coulnd't be initialized");
  assert(device, "Device coulnd't be initialized");
  assert(context, "Context coulnd't be initialized");
  
  ID3D11Resource* backBuffer = nullptr;
  swapchain->GetBuffer(0, __uuidof(ID3D11Resource), ((void**)&backBuffer));
  device->CreateRenderTargetView(backBuffer, nullptr, &target);
  if(backBuffer) backBuffer->Release();

  D3D11_DEPTH_STENCIL_DESC depthDesc = {};
  depthDesc.DepthEnable = TRUE;
  depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

  ID3D11DepthStencilState* depthStencilState;
  device->CreateDepthStencilState(&depthDesc, &depthStencilState);
  context->OMSetDepthStencilState(depthStencilState, 1);

  ID3D11Texture2D* depthStencilTexture;
  D3D11_TEXTURE2D_DESC depthTextureDesc = {};
  depthTextureDesc.Width = windowWidth;
  depthTextureDesc.Height = windowHeight;
  depthTextureDesc.MipLevels = 1;
  depthTextureDesc.ArraySize = 1;
  depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
  depthTextureDesc.SampleDesc.Count = 1;
  depthTextureDesc.SampleDesc.Quality = 0;
  depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
  depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  device->CreateTexture2D(&depthTextureDesc, nullptr, &depthStencilTexture);

  D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
  dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Texture2D.MipSlice = 0;
  device->CreateDepthStencilView(depthStencilTexture, &dsvDesc, &dsv);

  depthStencilState->Release();
  depthStencilTexture->Release();

  char path[512];
  full_path(path, "res\\shaders\\VertexShader.hlsl");
  wchar_t vPath[strlen(path)+1];
  std::mbstowcs(vPath, path, strlen(path)+1);

  full_path(path, "res\\shaders\\FragmentShader.hlsl");
  wchar_t fPath[strlen(path)+1];
  std::mbstowcs(fPath, path, strlen(path)+1);

  d3d_assert(compile_vertex_shader(&Vblob, vPath));
  d3d_assert(compile_fragment_shader(&Fblob, fPath));

  renderInfo.context = context;
  renderInfo.swapchain = swapchain;
  renderInfo.device = device;
  renderInfo.target = target;
  renderInfo.dsv = dsv;
  renderInfo.Vblob = Vblob;
  renderInfo.Fblob = Fblob;

  renderObjects.vertexBuffers = (ID3D11Buffer**)malloc(sizeof(int32));
  renderObjects.indexBuffers = (ID3D11Buffer**)malloc(sizeof(int32));
  renderObjects.vertexBufferStrides = (uint32*)malloc(sizeof(int32));
  renderObjects.indexBufferSizes = (uint32*)malloc(sizeof(int32));
  renderObjects.faceColorBuffers = (ID3D11Buffer**)malloc(sizeof(int32));
  renderObjects.vertexShaders = (ID3D11VertexShader**)malloc(sizeof(int32));
  renderObjects.fragmentShaders = (ID3D11PixelShader**)malloc(sizeof(int32));
  renderObjects.inputLayouts = (ID3D11InputLayout**)malloc(sizeof(int32));
}

void create_vertex_buffer(ID3D11Buffer** vertexBuffer, const void* vertices, uint32 size, uint32 stride) {
  ID3D11Device* device = renderInfo.device;

  D3D11_BUFFER_DESC VBDesc = {};
  VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  VBDesc.Usage = D3D11_USAGE_DEFAULT;
  VBDesc.CPUAccessFlags = 0;
  VBDesc.ByteWidth = size;
  VBDesc.StructureByteStride = stride;

  D3D11_SUBRESOURCE_DATA VBData = {};
  VBData.pSysMem = vertices;

  d3d_assert(device->CreateBuffer(&VBDesc, &VBData, vertexBuffer));
}

void create_index_buffer(ID3D11Buffer** indexBuffer, const void* indices, uint32 size, uint32 stride) {
  ID3D11Device* device = renderInfo.device;
  
  D3D11_BUFFER_DESC IBDesc = {};
  IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  IBDesc.Usage = D3D11_USAGE_DEFAULT;
  IBDesc.CPUAccessFlags = 0;
  IBDesc.ByteWidth = size;
  IBDesc.StructureByteStride = stride;

  D3D11_SUBRESOURCE_DATA IBData = {};
  IBData.pSysMem = indices;

  d3d_assert(device->CreateBuffer(&IBDesc, &IBData, indexBuffer));
}

void create_constant_buffer(ID3D11Buffer** constantBuffer, const void* mat, uint32 size) {
  ID3D11Device* device = renderInfo.device;

  D3D11_BUFFER_DESC CBDesc = {};
  CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  CBDesc.Usage = D3D11_USAGE_DYNAMIC;
  CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  CBDesc.ByteWidth = size;
  CBDesc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA CBData = {};
  CBData.pSysMem = mat;

  d3d_assert(device->CreateBuffer(&CBDesc, &CBData, constantBuffer));
}

uint32 add_to_renderer(ID3D11Buffer* vertexBuffer,        ID3D11Buffer* indexBuffer,
		       ID3D11Buffer* faceColorBuffer,     ID3D11VertexShader* vertexShader,
		       ID3D11PixelShader* fragmentShader, ID3D11InputLayout* inputLayout,
		       uint32 vertexBufferStride,         uint32 indexBufferSize) {
  uint32 i = renderObjects.count;

  uint32 size = (i + 1) * sizeof(uint32);

  renderObjects.vertexBuffers = (ID3D11Buffer**)realloc(renderObjects.vertexBuffers, size);
  renderObjects.vertexBuffers[i] = vertexBuffer;

  renderObjects.indexBuffers = (ID3D11Buffer**)realloc(renderObjects.indexBuffers, size);
  renderObjects.indexBuffers[i] = indexBuffer;

  renderObjects.faceColorBuffers = (ID3D11Buffer**)realloc(renderObjects.faceColorBuffers, size);
  renderObjects.faceColorBuffers[i] = faceColorBuffer;

  renderObjects.vertexShaders = (ID3D11VertexShader**)realloc(renderObjects.vertexShaders, size);
  renderObjects.vertexShaders[i] = vertexShader;

  renderObjects.fragmentShaders = (ID3D11PixelShader**)realloc(renderObjects.fragmentShaders, size);
  renderObjects.fragmentShaders[i] = fragmentShader;

  renderObjects.inputLayouts = (ID3D11InputLayout**)realloc(renderObjects.inputLayouts, size);
  renderObjects.inputLayouts[i] = inputLayout;

  renderObjects.vertexBufferStrides = (uint32*)realloc(renderObjects.vertexBufferStrides, size);
  renderObjects.vertexBufferStrides[i] = vertexBufferStride;

  renderObjects.indexBufferSizes = (uint32*)realloc(renderObjects.indexBufferSizes, size);
  renderObjects.indexBufferSizes[i] = indexBufferSize;

  renderObjects.count++;
  return i;
}

void render_loop() {
  ID3D11DeviceContext* context = renderInfo.context;
  ID3D11RenderTargetView* target = renderInfo.target;
  ID3D11DepthStencilView* dsv = renderInfo.dsv;

  for(uint32 i = 0; i < renderObjects.count; i++) {
    ID3D11Buffer* vertexBuffer = renderObjects.vertexBuffers[i];
    uint32 vertexBufferStride = renderObjects.vertexBufferStrides[i];
    ID3D11Buffer* indexBuffer = renderObjects.indexBuffers[i];
    uint32 indexBufferSize = renderObjects.indexBufferSizes[i];
    ID3D11Buffer* faceColorBuffer = renderObjects.faceColorBuffers[i];
    ID3D11VertexShader* vertexShader = renderObjects.vertexShaders[i];
    ID3D11PixelShader* fragmentShader = renderObjects.fragmentShaders[i];
    ID3D11InputLayout* inputLayout = renderObjects.inputLayouts[i];
    uint32 offset = 0;

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    context->PSSetConstantBuffers(0, 1, &faceColorBuffer);


    Mat4 mat = 
      //mat4_scaling(scale) *
      //mat4_z_rotation(rotation) *
      //mat4_x_rotation(rotation) *
      mat4_translation({0.0f, 0.0f, 4.0f}) *
      mat4_perspective(1.0f, 3.0f/4.0f, 0.5f, 10.0f); //@ToDo: abstract to camera
    
    mat = mat4_transpose(mat);
    
    ID3D11Buffer* translationBuffer;
    create_constant_buffer(&translationBuffer, &mat, sizeof(mat));
    context->VSSetConstantBuffers(0, 1, &translationBuffer);


    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(fragmentShader, nullptr, 0);
    context->IASetInputLayout(inputLayout);
    context->OMSetRenderTargets(1, &target, dsv);  // < might be a problem
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    context->DrawIndexed(indexBufferSize, 0, 0);  
    translationBuffer->Release();
  }
}

void draw_triangle() {
  ID3D11Device* device = renderInfo.device;
  ID3DBlob* Vblob = renderInfo.Vblob;
  ID3DBlob* Fblob = renderInfo.Fblob;  

  //vertex buffer
  float32 vertices[] = {
    -1.0f, -1.0f, -1.0f, 
     1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f 
  };
  
  ID3D11Buffer* vertexBuffer;
  uint32 stride = sizeof(float32) * 3;
  create_vertex_buffer(&vertexBuffer, &vertices, sizeof(vertices), stride);

  //index buffer
  uint16 indices[] = {
     0, 2, 1,  2, 3, 1,
     1, 3, 5,  3, 7, 5,
     2, 6, 3,  3, 6, 7,
     4, 5, 7,  4, 7, 6,
     0, 4, 2,  2, 4, 6,
     0, 1, 4,  1, 5, 4
  };

  ID3D11Buffer* indexBuffer;
  create_index_buffer(&indexBuffer, &indices, sizeof(indices), sizeof(uint16));

  float32 faceColors[] = {
     1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, 0.0f, 0.0f, 1.0f,
     0.0f, 1.0f, 0.0f, 1.0f,
     0.0f, 0.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 0.0f, 1.0f,
     0.0f, 1.0f, 1.0f, 1.0f
  };
  
  ID3D11Buffer* faceColorBuffer;
  create_constant_buffer(&faceColorBuffer, &faceColors, sizeof(faceColors));
   
  ID3D11VertexShader* vertexShader = nullptr;
  ID3D11PixelShader* fragmentShader = nullptr;
  d3d_assert(device->CreateVertexShader(Vblob->GetBufferPointer(), Vblob->GetBufferSize(), nullptr, &vertexShader));
  d3d_assert(device->CreatePixelShader(Fblob->GetBufferPointer(), Fblob->GetBufferSize(), nullptr, &fragmentShader));

  
  ID3D11InputLayout* inputLayout = nullptr;
  const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
    { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  d3d_assert(device->CreateInputLayout(inputElementDesc, sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
				       Vblob->GetBufferPointer(), Vblob->GetBufferSize(), &inputLayout));

  add_to_renderer(
    vertexBuffer, indexBuffer,
    faceColorBuffer, vertexShader,
    fragmentShader, inputLayout,
    stride, sizeof(indices) / sizeof(uint16));
}

void refresh_viewport(int32 x, int32 y, uint32 w, uint32 h) {
  ID3D11DeviceContext* context = renderInfo.context;
  D3D11_VIEWPORT* viewport = renderInfo.viewport;
  
  assert(context);
  if(!viewport) {
    viewport = (D3D11_VIEWPORT*)malloc(sizeof(D3D11_VIEWPORT));
    renderInfo.viewport = viewport;
  }
  
  assert(viewport);
  viewport->TopLeftX = x;
  viewport->TopLeftY = y;
  viewport->Width = w;
  viewport->Height = h;
  viewport->MinDepth = 0;
  viewport->MaxDepth = 1;
  context->RSSetViewports(1, viewport);
}

void swap_buffers(bool32 vSync) {
  IDXGISwapChain* swapchain = renderInfo.swapchain;
  swapchain->Present(vSync, 0);
}

void clear_buffer(float32 r, float32 g, float32 b, float32 a) {
  ID3D11DeviceContext* context = renderInfo.context;
  ID3D11RenderTargetView* target = renderInfo.target;  
  ID3D11DepthStencilView* dsv = renderInfo.dsv;  

  float32 color[] = { r, g , b, a };
  context->ClearRenderTargetView(target, color);
  context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
