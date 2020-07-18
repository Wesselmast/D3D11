cbuffer Light {
  float3 lightPos;
};

static const float3 materialColor = { 0.7f, 0.7f, 0.9f };
static const float3 ambientColor = { 0.15f, 0.15f, 0.15f };
static const float3 diffuseColor = { 0.1f, 0.1f, 0.1f };
static const float diffuseIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.045f;
static const float attQuad = 0.0075f;

float4 main(float3 world : Position, float3 normal : Normal) : SV_Target {
  const float3 vToL = lightPos - world;
  const float distToL = length(vToL);
  const float3 dirToL = vToL / distToL;

  const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
  const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, normal));
  return float4(saturate((diffuse + ambientColor) * materialColor), 1.0f);
}


/* Texture2D tex; */
/* SamplerState s; */

/* float4 main(float2 texcoord : TexCoord) : SV_Target { */
/*   return tex.Sample(s, texcoord); */
/* } */
