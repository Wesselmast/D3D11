cbuffer CBuf {
  matrix transform;
};

struct VSOut {
  float2 texcoord : TexCoord;
  float4 pos      : SV_Position;
};

VSOut main(float3 pos : Position, float2 texcoord : TexCoord) {
  VSOut vsout;
  vsout.pos = mul(float4(pos, 1.0f), transform);
  vsout.texcoord = texcoord;
  return vsout;
}