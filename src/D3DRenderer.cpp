#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

void d3d_assert(HRESULT err) {
  if(FAILED(err)) {
    printf("D3D ERROR: %d\n", (uint32)err);
    assert(false, "D3D ERROR, check log");
  }
}

HRESULT compile_shader(LPCWSTR path, LPCSTR target, ID3DBlob** blob) {
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

HRESULT compile_vertex_shader(ID3DBlob** blob, LPCWSTR path) {
  return compile_shader(path, "vs_5_0", blob);
}

HRESULT compile_fragment_shader(ID3DBlob** blob, LPCWSTR path) {
  return compile_shader(path, "ps_5_0", blob);
}

void draw_triangle(ID3D11DeviceContext* context, ID3D11Device* device, ID3D11RenderTargetView* target, D3D11_VIEWPORT* viewport, ID3DBlob** Vblob, ID3DBlob** Fblob) {
  float32 vertices[] = {
     0.0f,  0.5f,
     0.5f, -0.5f,
    -0.5f, -0.5f
  };
  
  ID3D11Buffer* vertexBuffer;

  D3D11_BUFFER_DESC VBDesc = {};
  VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  VBDesc.Usage = D3D11_USAGE_DEFAULT;
  VBDesc.CPUAccessFlags = 0;
  VBDesc.ByteWidth = sizeof(vertices);
  VBDesc.StructureByteStride = sizeof(float32) * 2;

  D3D11_SUBRESOURCE_DATA VBData = {};
  VBData.pSysMem = vertices;

  d3d_assert(device->CreateBuffer(&VBDesc, &VBData, &vertexBuffer));

  const UINT stride = sizeof(float32) * 2;
  const UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

  ID3D11VertexShader* vertexShader = nullptr;
  ID3D11PixelShader* fragmentShader = nullptr;
  d3d_assert(device->CreateVertexShader((*Vblob)->GetBufferPointer(), (*Vblob)->GetBufferSize(), nullptr, &vertexShader));
  d3d_assert(device->CreatePixelShader((*Fblob)->GetBufferPointer(), (*Fblob)->GetBufferSize(), nullptr, &fragmentShader));

  context->VSSetShader(vertexShader, nullptr, 0);
  context->PSSetShader(fragmentShader, nullptr, 0);
  
  ID3D11InputLayout* inputLayout = nullptr;
  const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
    { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  d3d_assert(device->CreateInputLayout(inputElementDesc, sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
				       (*Vblob)->GetBufferPointer(), (*Vblob)->GetBufferSize(), &inputLayout));

  context->IASetInputLayout(inputLayout);
  context->OMSetRenderTargets(1, &target, nullptr);  // < might be a problem
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  context->RSSetViewports(1, viewport);

  context->Draw(3, 0);

  
  vertexBuffer->Release();
  vertexShader->Release();
  fragmentShader->Release();
  inputLayout->Release();
}

void refresh_viewport(int32 x, int32 y, uint32 w, uint32 h) {
}

void swap_buffers(IDXGISwapChain* swapchain) {
  swapchain->Present(1, 0);
}

void clear_buffer(ID3D11DeviceContext* context, ID3D11RenderTargetView* target, float32 r, float32 g, float32 b, float32 a) {
  float32 color[] = { r, g , b, a };
  context->ClearRenderTargetView(target, color);
}
