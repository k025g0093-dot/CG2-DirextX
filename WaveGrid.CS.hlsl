// WaveSim.CS.hlsl

cbuffer WaveParams : register(b0)
{
      float gTime; // CPU: t += 0.01f
      float gWaveFreq; // 30.0f (from ImGui)
      float gWaveStrength; // 10.0f (from ImGui)
      float gDamping; // 0.993f
    
      float gMul;
      uint gWidth;
      uint gHeight;
      uint gPad;
};

StructuredBuffer<uint> gWall : register(t0); // SRV: 読み専用
RWStructuredBuffer<float> gCurr : register(u0); // UAV
RWStructuredBuffer<float> gPrev : register(u1); // UAV
RWStructuredBuffer<float> gNext : register(u2); // UAV

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
      if (id.x >= gWidth || id.y >= gHeight)
            return;
      int index = id.y * gWidth + id.x;

      int idx = id.y * gWidth + id.x;
      
      if (id.x == 1)  // 左端
      {
            gCurr[idx] = sin(gTime * gWaveFreq) * gWaveStrength;
      }
      
    // 壁 or 端 → 波をゼロに
      if (gWall[index] != 0 ||
        id.x == 0 || id.x == gWidth - 1 ||
        id.y == 0 || id.y == gHeight - 1)
      {
            gNext[index] = 0;
            return;
      }


      
      float u = gCurr[index];
      float uPre = gPrev[index];
      float uL = gCurr[index - 1];
      float uR = gCurr[index + 1];
      float uT = gCurr[(id.y - 1) * gWidth + id.x];
      float uB = gCurr[(id.y + 1) * gWidth + id.x];

      gNext[index] = gDamping * (
        u + u - uPre +
        gMul * (-4 * u + uL + uR + uT + uB)
    );
}