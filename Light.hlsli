struct Light
{
    float3 dirOrPos;
    float3 color;
    float intensity;
};

StructuredBuffer<Light> g_lights : register(t3);
