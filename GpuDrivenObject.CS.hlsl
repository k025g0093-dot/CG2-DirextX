// =============================================================
//  GpuDrivenObject.CS.hlsl
//  RawTransform（pos/rot/scale）から World・WVP 行列を計算して
//  InstanceData バッファに書き込む Compute Shader
// =============================================================

// ------- 構造体（C++ 側と完全一致させること） -------

struct RawTransform
{
      float3 pos;
      float pad0;
      float3 rot;
      float pad1;
      float3 scale;
      float pad2;
};

struct InstanceData
{
      row_major float4x4 WVP;
      row_major float4x4 World;
};

// ------- 定数バッファ -------

cbuffer Params : register(b0)
{
      row_major float4x4 gViewProj;
      int gInstanceCount;
      float pad[3];
};

// ------- リソース -------

StructuredBuffer<RawTransform> gTransforms : register(t0); // 読む（SRV）
RWStructuredBuffer<InstanceData> gInstances : register(u0); // 書く（UAV）

// ------- 行列生成関数（C++ の MakeXXX と対応） -------

float4x4 MakeScaleMatrix(float3 s)
{
      return float4x4(
        s.x, 0, 0, 0,
        0, s.y, 0, 0,
        0, 0, s.z, 0,
        0, 0, 0, 1
    );
}

float4x4 MakeRotateXMatrix(float r)
{
      float c = cos(r);
      float s = sin(r);
      return float4x4(
        1, 0, 0, 0,
        0, c, s, 0,
        0, -s, c, 0,
        0, 0, 0, 1
    );
}

float4x4 MakeRotateYMatrix(float r)
{
      float c = cos(r);
      float s = sin(r);
      return float4x4(
        c, 0, -s, 0,
        0, 1, 0, 0,
        s, 0, c, 0,
        0, 0, 0, 1
    );
}

float4x4 MakeRotateZMatrix(float r)
{
      float c = cos(r);
      float s = sin(r);
      return float4x4(
        c, s, 0, 0,
        -s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

float4x4 MakeTranslateMatrix(float3 t)
{
      return float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        t.x, t.y, t.z, 1
    );
}

// C++ 側の MakeAffineMatrix(scale, rotate, translate) と同じ合成順
float4x4 MakeAffineMatrix(float3 scale, float3 rot, float3 pos)
{
      float4x4 S = MakeScaleMatrix(scale);
      float4x4 RX = MakeRotateXMatrix(rot.x);
      float4x4 RY = MakeRotateYMatrix(rot.y);
      float4x4 RZ = MakeRotateZMatrix(rot.z);
      float4x4 T = MakeTranslateMatrix(pos);

    // scale → rotX → rotY → rotZ → translate
      return mul(S, mul(RX, mul(RY, mul(RZ, T))));
}


// ------- エントリーポイント -------

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // 範囲外スレッドは早期リターン
      if ((int) id.x >= gInstanceCount)
            return;

      RawTransform t = gTransforms[id.x];

    // World 行列を作る
      float4x4 world = MakeAffineMatrix(t.scale, t.rot, t.pos);

    // WVP 行列を作る（C++ の Multiply(world, viewProj) と同じ）
      float4x4 wvp = mul(world, gViewProj);

    // InstanceData バッファに書き込む
      gInstances[id.x].World = world;
      gInstances[id.x].WVP = wvp;
}
