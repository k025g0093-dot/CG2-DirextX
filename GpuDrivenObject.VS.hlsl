#include "GpuDrivenObject.hlsli"

struct InstanceData
{
      row_major float4x4 WVP;
      row_major float4x4 World;
};

StructuredBuffer<InstanceData> gInstances : register(t2);

struct DrawInfo
{
      uint baseInstance;
};

ConstantBuffer<DrawInfo> gDrawInfo : register(b2);

struct VertexShaderInput
{
      float4 position : POSITION;
      float2 texcoord : TEXCOORD;
      float3 normal : NORMAL0;
      float3 tangent : TANGENT0;
};


VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
      InstanceData instance = gInstances[gDrawInfo.baseInstance + instanceId];
    
      VertexShaderOutput output;
      output.position = mul(input.position, instance.WVP);
      output.normal = normalize(mul(input.normal, (float3x3) instance.World));
      output.texcoord = input.texcoord;
      output.tangent = normalize(mul(input.tangent, (float3x3) instance.World));
      output.worldPosition = mul(input.position, instance.World).xyz;
      return output;
}