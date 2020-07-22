cbuffer WorldSpace {
  matrix viewProj;
};

cbuffer ObjectSpace {
  matrix model;
};

struct VSOut {
  /* float2 texcoord : TexCoord; */
  /* float4 pos      : SV_Position; */
  float3 world : Position;
  float3 normal : Normal;
  float2 texcoord : TexCoord;
  float4 pos : SV_Position;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 texcoord : TexCoord) {//float2 texcoord : TexCoord) {
/*   VSOut vsout; */
/*   vsout.pos = mul(float4(pos, 1.0f), transform); */
/*   vsout.texcoord = texcoord; */
/*   return vsout; */
/* } */

  VSOut vsout;
  vsout.world = (float3)mul(float4(pos, 1.0f), model);
  vsout.normal = mul(normal, (float3x3)model);
  vsout.texcoord = texcoord;
  matrix MVP = mul(model, viewProj);
  vsout.pos = mul(float4(pos, 1.0f), MVP); 
  return vsout;
}
