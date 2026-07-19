#include "GpuDrivenObject.hlsli"
#include "Light.hlsli"

#define float32_t4 float4
#define float32_t4x4 float4x4

struct Material
{
    float32_t4 color;
      int enableLighting;
      int enableNormalMap;
      float2 padding;
    float32_t4x4 uvTransform;
};

struct Camera
{
      float3 worldPorition;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

cbuffer MaterialBuffer : register(b0)
{
      Material gMaterial;
};

cbuffer CameraBuffer : register(b2)
{
      Camera gCamera;
};

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t4> gNormalTexture : register(t1);
SamplerState gSampler : register(s0);

static const uint LIGHT_COUNT = 8;

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

      if (gMaterial.enableLighting != 0)
      {
            float3 toEye = normalize(gCamera.worldPorition - input.worldPosition);
            float3 baseColor = gMaterial.color.rgb * textureColor.rgb;

            float3 finalColor = float3(0.0f, 0.0f, 0.0f);
            [loop]
            for (uint i = 0; i < LIGHT_COUNT; i++)
            {
                Light light = g_lights[i];
                float NdotL = dot(normal, -normalize(light.dirOrPos));
                float halfLambert = pow(NdotL * 0.5f + 0.5f, 2.0f);

                float3 diffuse = baseColor * light.color * halfLambert * light.intensity;

                float3 halfVector = normalize(-normalize(light.dirOrPos) + toEye);
                float NDotH = dot(normal, halfVector);
                float specularPow = pow(saturate(NDotH), 32.0f);
                float3 specular = light.color * light.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

                finalColor += diffuse + specular;
            }

            output.color.rgb = finalColor;
            output.color.a = gMaterial.color.a * textureColor.a;
      }
      else
      {
            output.color = gMaterial.color * textureColor;
      }

      return output;
}