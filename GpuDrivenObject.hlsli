struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 worldPosition : POSITION0;
};