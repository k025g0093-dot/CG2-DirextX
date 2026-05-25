#define float32_t4 float4
#include "Object3d.hlsli"


struct TransformationMatrix
{
      row_major float4x4 WVP;
      row_major float4x4 World;

};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1); // b1に合わせる

struct VertexShaderInput
{
    float32_t4 position : POSITION;
      float2 texcoord : TEXCOORD;
      float3 normal : NORMAL0;
};


VertexShaderOutput main(VertexShaderInput input)
{
      VertexShaderOutput output;
    // ここで mul を使って行列を掛ける！
      output.position = mul(input.position, gTransformationMatrix.WVP);
      output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.World));
      output.texcoord = input.texcoord;

      return output;
}