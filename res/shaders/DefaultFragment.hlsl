cbuffer Light {
  float3 lightPos;
  float3 ambientColor;
  float3 diffuseColor;
  float diffuseIntensity;
  float attConst;
  float attLin;
  float attQuad;
};

cbuffer Material {
  float3 materialColor;
  float2 tiling;
}

Texture2D tex;
SamplerState s;

float4 main(float3 world : Position, float3 normal : Normal, float2 texcoord : TexCoord) : SV_Target {
  const float3 vToL = lightPos - world;
  const float distToL = length(vToL);
  const float3 dirToL = vToL / distToL;

  const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
  const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, normal));
  
  return float4(saturate((diffuse + ambientColor) * materialColor * tex.Sample(s, texcoord * tiling)), 1.0f);
}

/* float4 main(float2 texcoord : TexCoord) : SV_Target { */
/*   return tex.Sample(s, texcoord); */
/* } */
