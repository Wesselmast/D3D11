#include "Camera.cpp"

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

struct RenderInfo {
  ID3D11DeviceContext* context = nullptr;
  IDXGISwapChain* swapchain = nullptr;
  ID3D11Device* device = nullptr;
  ID3D11RenderTargetView* target = nullptr;
  ID3D11DepthStencilView* dsv = nullptr;
};

const uint32 RENDER_OBJECT_LIMIT = 1000;

struct LightBuffer {
  alignas(16) Vec3 position = { 0.0f, 0.0f, 0.0f };
  alignas(16) Vec3 ambientColor = { 0.15f, 0.15f, 0.15f };
  alignas(16) Vec3 diffuseColor = { 0.1f, 0.1f, 0.1f };
  float32 diffuseIntensity = 1.0f;
  float32 constantAttenuation = 1.0f;
  float32 linearAttenuation = 0.045f;
  float32 quadradicAttenuation = 0.0075f;
};

struct Material {
  alignas(16) Vec3 materialColor = { 0.7f, 0.7f, 0.9f };;
};

struct RenderObjects {
  ID3D11Buffer* vertexBuffers[RENDER_OBJECT_LIMIT];
  uint32 vertexBufferStrides[RENDER_OBJECT_LIMIT];
  ID3D11Buffer* indexBuffers[RENDER_OBJECT_LIMIT];
  uint32 indexBufferSizes[RENDER_OBJECT_LIMIT];
  ID3D11VertexShader* vertexShaders[RENDER_OBJECT_LIMIT];
  ID3D11PixelShader* fragmentShaders[RENDER_OBJECT_LIMIT];
  ID3D11InputLayout* inputLayouts[RENDER_OBJECT_LIMIT];
  ID3D11ShaderResourceView* resourceViews[RENDER_OBJECT_LIMIT];
  ID3D11SamplerState* samplers[RENDER_OBJECT_LIMIT];
  ID3D11Buffer* transformBuffers[RENDER_OBJECT_LIMIT];
  ID3D11Buffer* materialBuffers[RENDER_OBJECT_LIMIT];
  uint32 count = 0;
};
 
static RenderInfo renderInfo; // @CleanUp: Maybe have this not be a global variable

#define d3d_assert(condition) \
  if(FAILED(condition)) { \
    log_("D3D ERROR: %d\n", (uint32)condition); \
    log_("\n"); \
    exit(1); \
  }

static HRESULT compile_shader(LPCWSTR path, LPCSTR target, ID3DBlob** blob) {
  ID3DBlob* errorBlob;

  HRESULT result = D3DCompileFromFile(path, 0, 0, "main", target, 0, 0, blob, &errorBlob );
  if(FAILED(result)) {
    if(errorBlob) {
      log_("%s", (char*)errorBlob->GetBufferPointer());
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
  assert(renderInfo.dsv, "depthstencil has not been initialized!");
}

void init_renderer(HWND window) {
  ID3D11Device* device = nullptr;
  IDXGISwapChain* swapchain = nullptr;
  ID3D11DeviceContext* context = nullptr;
  ID3D11RenderTargetView* target = nullptr;
  ID3D11DepthStencilView* dsv = nullptr;

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

  renderInfo.context = context;
  renderInfo.swapchain = swapchain;
  renderInfo.device = device;
  renderInfo.target = target;
  renderInfo.dsv = dsv;
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

void create_texture(ID3D11ShaderResourceView** view, Bitmap* bmp) {
  ID3D11Device* device = renderInfo.device;

  D3D11_TEXTURE2D_DESC textureDesc = {};
  textureDesc.Width = bmp->width;
  textureDesc.Height = bmp->height;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.SampleDesc.Quality = 0;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;
  
  D3D11_SUBRESOURCE_DATA textureData = {};
  textureData.pSysMem = bmp->memory;
  textureData.SysMemPitch = bmp->width * 4;
    
  ID3D11Texture2D* texture;
  d3d_assert(device->CreateTexture2D(&textureDesc, &textureData, &texture));
  
  D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
  viewDesc.Format = textureDesc.Format;
  viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  viewDesc.Texture2D.MostDetailedMip = 0;
  viewDesc.Texture2D.MipLevels = 1;
  
  d3d_assert(device->CreateShaderResourceView(texture, &viewDesc, view));
  texture->Release();
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

void set_object_transform(RenderObjects* renderObjects, uint32 index, Vec3 position, Vec3 rotation, Vec3 scale) {
    Mat4 mat = 
      mat4_transpose(
      mat4_scaling(scale) *
      mat4_euler_rotation(rotation) *
      mat4_translation(position));    

    ID3D11Buffer* transformBuffer;
    create_constant_buffer(&transformBuffer, &mat, sizeof(Mat4));
    if(renderObjects->transformBuffers[index]) (renderObjects->transformBuffers[index])->Release();
    renderObjects->transformBuffers[index] = transformBuffer;
}  

void set_object_texture(RenderObjects* renderObjects, uint32 index, Bitmap* bmp) {    
    ID3D11ShaderResourceView* resourceView;
    create_texture(&resourceView, bmp);
    if(renderObjects->resourceViews[index]) (renderObjects->resourceViews[index])->Release();
    renderObjects->resourceViews[index] = resourceView;
}

void set_object_material(RenderObjects* renderObjects, uint32 index, Material* material) {
  ID3D11Buffer* materialBuffer;
  create_constant_buffer(&materialBuffer, material, sizeof(Material));
  if(renderObjects->materialBuffers[index]) (renderObjects->materialBuffers[index])->Release();
  renderObjects->materialBuffers[index] = materialBuffer;
}

void update_view_projection(const Mat4& viewProjection) {
  ID3D11DeviceContext* context = renderInfo.context;

  Mat4 tVP = mat4_transpose(viewProjection);
  ID3D11Buffer* viewProjectionBuffer;
  create_constant_buffer(&viewProjectionBuffer, &tVP, sizeof(Mat4));
  context->VSSetConstantBuffers(0, 1, &viewProjectionBuffer);
  viewProjectionBuffer->Release();
}

void update_render_targets() {
  ID3D11DeviceContext* context = renderInfo.context;
  ID3D11RenderTargetView* target = renderInfo.target;
  ID3D11DepthStencilView* dsv = renderInfo.dsv;
  
  context->OMSetRenderTargets(1, &target, dsv);
}

void render_loop(RenderObjects* renderObjects, const Mat4& viewProjection, const Vec3& DEBUGVec) {
  ID3D11DeviceContext* context = renderInfo.context;
 
  update_render_targets();
  update_view_projection(viewProjection);
 
  LightBuffer light = {};
  light.position = DEBUGVec;
  ID3D11Buffer* lightBuffer;
  create_constant_buffer(&lightBuffer, &light, sizeof(LightBuffer));
  context->PSSetConstantBuffers(0, 1, &lightBuffer);
  lightBuffer->Release();
 
  for(uint32 i = 0; i < renderObjects->count; i++) {
    ID3D11Buffer* vertexBuffer = renderObjects->vertexBuffers[i];
    uint32 vertexBufferStride = renderObjects->vertexBufferStrides[i];
    ID3D11Buffer* indexBuffer = renderObjects->indexBuffers[i];
    ID3D11Buffer* transformBuffer = renderObjects->transformBuffers[i];
    uint32 indexBufferSize = renderObjects->indexBufferSizes[i];
    ID3D11ShaderResourceView* resourceView = renderObjects->resourceViews[i];
    ID3D11VertexShader* vertexShader = renderObjects->vertexShaders[i];
    ID3D11PixelShader* fragmentShader = renderObjects->fragmentShaders[i];
    ID3D11SamplerState* sampler = renderObjects->samplers[i];
    ID3D11InputLayout* inputLayout = renderObjects->inputLayouts[i];
    ID3D11Buffer* materialBuffer = renderObjects->materialBuffers[i];
    uint32 offset = 0;

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    if(resourceView) {
      assert(sampler, "In order to display a resource view the renderer needs a sampler!");
      context->PSSetSamplers(0, 1, &sampler);
      context->PSSetShaderResources(0, 1, &resourceView);
    }

    context->PSSetConstantBuffers(1, 1, &materialBuffer);
    context->VSSetConstantBuffers(1, 1, &transformBuffer);
  
    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(fragmentShader, nullptr, 0);
    context->IASetInputLayout(inputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    context->DrawIndexed(indexBufferSize, 0, 0);  
  }
}

uint32 create_object(RenderObjects* renderObjects, ModelInfo* info) {
  ID3D11Device* device = renderInfo.device;
  float32* vertices = info->vertices;
  uint32 vSize = info->vSize;
  uint32 stride = info->stride;
  uint16* indices = info->indices;
  uint32 iSize = info->iSize;
  uint32 index = renderObjects->count;

  assert(index < RENDER_OBJECT_LIMIT, "Cannot create more than %d objects!", RENDER_OBJECT_LIMIT);

  ID3D11Buffer* vertexBuffer;
  create_vertex_buffer(&vertexBuffer, vertices, vSize, stride);
  renderObjects->vertexBuffers[index] = vertexBuffer;
  renderObjects->vertexBufferStrides[index] = stride;

  ID3D11Buffer* indexBuffer;
  create_index_buffer(&indexBuffer, indices, iSize, sizeof(uint16));
  renderObjects->indexBuffers[index] = indexBuffer;
  renderObjects->indexBufferSizes[index] = iSize / sizeof(uint16);

  char path[512];
  ID3DBlob* vBlob;
  full_path(path, "res\\shaders\\DefaultVertex.hlsl");
  wchar_t vPath[strlen(path)+1];
  std::mbstowcs(vPath, path, strlen(path)+1);
  d3d_assert(compile_vertex_shader(&vBlob, vPath));

  ID3DBlob* fBlob;
  full_path(path, "res\\shaders\\DefaultFragment.hlsl");
  wchar_t fPath[strlen(path)+1];
  std::mbstowcs(fPath, path, strlen(path)+1);
  d3d_assert(compile_fragment_shader(&fBlob, fPath));

  ID3D11VertexShader* vertexShader = nullptr;
  d3d_assert(device->CreateVertexShader(vBlob->GetBufferPointer(), vBlob->GetBufferSize(), nullptr, &vertexShader));
  renderObjects->vertexShaders[index] = vertexShader;

  ID3D11PixelShader* fragmentShader = nullptr;
  d3d_assert(device->CreatePixelShader(fBlob->GetBufferPointer(), fBlob->GetBufferSize(), nullptr, &fragmentShader));
  renderObjects->fragmentShaders[index] = fragmentShader;

  ID3D11SamplerState* sampler;
  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  d3d_assert(device->CreateSamplerState(&samplerDesc, &sampler));
  renderObjects->samplers[index] = sampler;

  ID3D11InputLayout* inputLayout = nullptr;
  const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
    { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "Normal",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  uint32 inputCount = sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
  d3d_assert(device->CreateInputLayout(inputElementDesc, inputCount, vBlob->GetBufferPointer(), vBlob->GetBufferSize(), &inputLayout));
  renderObjects->inputLayouts[index] = inputLayout;

  renderObjects->resourceViews[index] = nullptr;
  set_object_transform(renderObjects, index, vec3_from_scalar(0.0f), vec3_from_scalar(0.0f), vec3_from_scalar(1.0f));

  Material mat = {};
  set_object_material(renderObjects, index, &mat); 

  vBlob->Release();
  fBlob->Release();
 
  renderObjects->count++;
  return index;
}

uint32 create_cube(RenderObjects* renderObjects) {
  float32 vertices[] = {
    //near
    -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
     1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f,  1.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
     
    //far
    -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f,  1.0f,
 
    //left
    -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f,  1.0f,

    //right
     1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f,  1.0f,

    //bottom
    -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f,  0.0f,
     1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f,  1.0f,
     1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f,  1.0f,

    //top
    -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f,  1.0f,
  };  

  uint16 indices[] = {
     0,  2,  1,  2,  3,  1,
     4,  5,  7,  4,  7,  6,
     8,  10, 9,  10, 11, 9,
     12, 13, 15, 12, 15, 14,
     16, 17, 18, 18, 17, 19,
     20, 23, 21, 20, 22, 23
  };

  ModelInfo info = {};
  info.vertices = &vertices[0];
  info.stride = sizeof(float32) * 8;
  info.vSize = sizeof(vertices);
  info.indices = &indices[0];
  info.iSize = sizeof(indices);
  
  return create_object(renderObjects, &info);
}

uint32 create_plane(RenderObjects* renderObjects) {
  float32 vertices[] = {
    -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
    -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
     1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
  };

  uint16 indices[] = {
     1, 2, 0,  1, 3, 2
  };

  ModelInfo info = {};
  info.vertices = &vertices[0];
  info.stride = sizeof(float32) * 8;
  info.vSize = sizeof(vertices);
  info.indices = &indices[0];
  info.iSize = sizeof(indices);

  return create_object(renderObjects, &info);
}

uint32 create_model(RenderObjects* renderObjects, ModelInfo info) {
  return create_object(renderObjects, &info);
}

void refresh_viewport(int32 x, int32 y, uint32 w, uint32 h) {
  ID3D11DeviceContext* context = renderInfo.context;
  D3D11_VIEWPORT viewport = {};
  viewport.TopLeftX = x;
  viewport.TopLeftY = y;
  viewport.Width = w;
  viewport.Height = h;
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  context->RSSetViewports(1, &viewport);
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
