#define float32_t4 float4



struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;

};