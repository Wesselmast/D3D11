Texture2D tex;
SamplerState s;

float4 main(float2 texcoord : TexCoord) : SV_Target {
  return tex.Sample(s, texcoord);
}