struct VSout {
  float3 col : Color;
  float4 pos : SV_Position;
};

cbuffer CBuf {
  matrix transform;
};

VSout main(float3 pos : Position, float3 color : Color) {
  VSout vso;
  vso.pos = mul(float4(pos, 1.0f), transform);   
  vso.col = color;  
  return vso;
}