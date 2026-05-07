#define float32_t4 float4

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

// ピクセルシェーダーのメイン
PixelShaderOutput main()
{
    PixelShaderOutput output;
    
    output.color = gMaterial.color;
    
    return output;
}