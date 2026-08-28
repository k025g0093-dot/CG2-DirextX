#include "GpuDrivenObject.hlsli"

StructuredBuffer<InstanceData> gInstances : register(t2);

cbuffer DrawInfoBuffer : register(b2)
{
      uint baseInstance;
};

cbuffer LightVPBuffer : register(b4)
{
      row_major float4x4 gLightVP;
};

struct VertexShaderInput
{
      float4 position : POSITION;
      float2 texcoord : TEXCOORD;
      float3 normal : NORMAL0;
      float3 tangent : TANGENT0;
};

float4 main(VertexShaderInput input, uint instanceId : SV_InstanceID) : SV_POSITION
{
      InstanceData instance = gInstances[baseInstance + instanceId];
      float4 pos = float4(input.position.xyz, 1.0f);
      pos = mul(pos, instance.World);
      pos = mul(pos, gLightVP);
      return pos;
}