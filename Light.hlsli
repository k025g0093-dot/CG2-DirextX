struct Light
{
    float3 dirOrPos;
    int type;
    float4 color;
    float intensity;
};

StructuredBuffer<Light> g_lights : register(t3);
