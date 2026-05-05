#define float32_t4 float4

// --- 頂点シェーダー関連 ---
struct VertexShaderInput
{
    float32_t4 position : POSITION;
};

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
};

// 頂点シェーダーのメイン
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = input.position;
    return output;
}
