cbuffer CBuf {
  float4 faceColors[6];
}

float4 main(uint tid : SV_PrimitiveID) : SV_Target {
  return faceColors[tid / 2]; //2 because a cube has two triangles per face
}