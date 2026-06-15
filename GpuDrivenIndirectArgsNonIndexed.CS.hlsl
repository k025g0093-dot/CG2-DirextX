// GpuDrivenIndirectArgsNonIndexed.CS.hlsl

struct NonIndexedBatchInfo
{
      uint baseInstance;
      uint instanceCount;

      uint vertexCountPerInstance;
      uint startVertexLocation;

      uint pad0;
      uint pad1;
      uint pad2;
      uint pad3;
};

struct DrawIndirectCommand
{
    // CommandSignature:
    // 0: ROOT_CONSTANT b2, 1 uint
    // 1: DRAW
      uint baseInstance;

      uint vertexCountPerInstance;
      uint instanceCount;
      uint startVertexLocation;
      uint startInstanceLocation;
};

cbuffer Params : register(b0)
{
      uint gBatchCount;
      uint3 pad;
};

StructuredBuffer<NonIndexedBatchInfo> gBatches : register(t0);
RWStructuredBuffer<DrawIndirectCommand> gCommands : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
      uint batchIndex = id.x;
      if (batchIndex >= gBatchCount)
      {
            return;
      }

      NonIndexedBatchInfo batch = gBatches[batchIndex];

      DrawIndirectCommand cmd;
      cmd.baseInstance = batch.baseInstance;

      cmd.vertexCountPerInstance = batch.vertexCountPerInstance;
      cmd.instanceCount = batch.instanceCount;
      cmd.startVertexLocation = batch.startVertexLocation;
      cmd.startInstanceLocation = 0;

      gCommands[batchIndex] = cmd;
}