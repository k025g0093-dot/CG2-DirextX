#define float32_t4 float4

// --- ピクセルシェーダー関連 ---
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0; // 名前を color にすると分かりやすいです
};

// ピクセルシェーダーのメイン
PixelShaderOutput main()
{
    PixelShaderOutput output;
    
    output.color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return output;
}