//cbuffer CBuf {
//  float4 faceColors[6];
//}

Texture2D tex;
SamplerState s;

float4 main(float2 texcoord : TexCoord) : SV_Target {
  return tex.Sample(s, texcoord);
  //return faceColors[tid / 2]; //2 because a cube has two triangles per face
}