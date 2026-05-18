#define float32_t4 float4
#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
      int enableLighting;
};

ConstantBuffer<Material> gMaterial : register(b0);

// --- ピクセルシェーダー関連 ---

struct DirectionalLight
{
      float32_t4 color;
      float3 direction;
      float intensity;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0; // 名前を color にすると分かりやすいです
};

Texture2D<float32_t4> gTexture : register(t0); // t0に合わせる
SamplerState gSampler : register(s0); // s0に合わせる
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);


// ピクセルシェーダーのメイン
PixelShaderOutput main(VertexShaderOutput input)
{
      PixelShaderOutput output;
    
     float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);

      if (gMaterial.enableLighting != 0)
      {
            float Ndotl = dot(normalize(input.normal), -gDirectionalLight.direction);
            float cos = pow(Ndotl * 0.5f + 0.5f, 2.0f);
            output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
      }
      else
      {
            output.color = gMaterial.color * textureColor;
      }

    
      return output;
}
