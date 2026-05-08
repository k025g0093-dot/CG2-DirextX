struct TransformationMatrix
{
    row_major float4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0); // b1に合わせる

struct VertexShaderInput
{
    float4 position : POSITION;
};
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    // ここで mul を使って行列を掛ける！
    output.position = mul(input.position, gTransformationMatrix.WVP);
    return output;
}