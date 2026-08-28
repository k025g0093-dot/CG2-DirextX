// WaveGrid.CS.hlsl

cbuffer WaveParams : register(b0)
{
      float gTime;
      float gWaveFreq;
      float gWaveStrength;
      float gDamping;
      float gMul;
      uint gWidth;
      uint gHeight;
      uint gPad;
};

//レジスタ関係
StructuredBuffer<uint> gWall : register(t0);
RWStructuredBuffer<float> gCurr : register(u0);
RWStructuredBuffer<float> gPrev : register(u1);
RWStructuredBuffer<float> gNext : register(u2);
RWStructuredBuffer<float4> gNormal : register(u3);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
      if (id.x >= gWidth || id.y >= gHeight)
            return;

      int index = id.y * gWidth + id.x;

    // 端 → 波ゼロ、法線(0,1,0)
      if (id.x == 0 || id.x == gWidth - 1 ||
        id.y == 0 || id.y == gHeight - 1)
      {
            gNext[index] = 0;
            gNormal[index] = float4(0, 1, 0, 0);
            return;
      }

      if (id.x == 1)
            gCurr[index] = sin(gTime * gWaveFreq) * gWaveStrength;

      float u = gCurr[index];
      float uPre = gPrev[index];
      float uL = gCurr[index - 1];
      float uR = gCurr[index + 1];
      float uT = gCurr[(id.y - 1) * gWidth + id.x];
      float uB = gCurr[(id.y + 1) * gWidth + id.x];

    // 法線（壁の有無に関わらず計算）
      float nx = uL - uR;
      float ny = 2.0;
      float nz = uT - uB;
      float3 n = normalize(float3(nx, ny, nz));
      gNormal[index] = float4(n, 0);

    // 波の更新
      gNext[index] = gDamping * (
        u + u - uPre +
        gMul * (-4 * u + uL + uR + uT + uB)
    );

    // 壁セルは高さだけ強制0
      if (gWall[index] != 0)
            gNext[index] = 0;
}