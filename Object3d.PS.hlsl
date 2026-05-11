#define float32_t4 float4
#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

// --- ピクセルシェーダー関連 ---



struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0; // 名前を color にすると分かりやすいです
};

Texture2D<float32_t4> gTexture : register(t0); // t0に合わせる
SamplerState gSampler : register(s0); // s0に合わせる



// ピクセルシェーダーのメイン
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
     float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    output.color = gMaterial.color * textureColor;
    
    return output;
}