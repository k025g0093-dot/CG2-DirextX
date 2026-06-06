#define float32_t4 float4
#define float32_t4x4 float4x4
#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
      int enableLighting;
      int enableNormalMap;
      float2 padding; // パディングを追加して16バイトの倍数にする
    float32_t4x4 uvTransform;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct DirectionalLight
{
    float32_t4 color;
      float3 direction;
      float intensity;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t4> gNormalTexture : register(t1);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{
      PixelShaderOutput output;

      float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    float32_t4 textureColor;
      if (gMaterial.enableLighting == -1)
      {
            textureColor = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
      }
      else
      {
            textureColor = gTexture.Sample(gSampler, transformedUV.xy);
      }

      float3 normal = normalize(input.normal);

      if (gMaterial.enableNormalMap != 0)
      {
            float3 tangent = normalize(input.tangent);

            tangent = normalize(tangent - normal * dot(normal, tangent));

            float3 bitangent = normalize(cross(normal, tangent));

            float3x3 TBN = float3x3(tangent, bitangent, normal);

            float3 sampledNormal = gNormalTexture.Sample(gSampler, transformedUV.xy).xyz;
            sampledNormal = sampledNormal * 2.0f - 1.0f;

            normal = normalize(mul(sampledNormal, TBN));
      }

      if (gMaterial.enableLighting != 0 && gMaterial.enableLighting != -1)
      {
            float NdotL = dot(normal, -normalize(gDirectionalLight.direction));
            float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);

            output.color =
            gMaterial.color *
            textureColor *
            gDirectionalLight.color *
            cos *
            gDirectionalLight.intensity;
      }
      else
      {
            output.color = gMaterial.color * textureColor;
      }

      return output;
}