// GpuDrivenIndirectArgs.CS.hlsl

struct BatchInfo
{
      uint baseInstance;
      uint instanceCount;

      uint indexCountPerInstance;
      uint startIndexLocation;
      int baseVertexLocation;

      uint pad0;
      uint pad1;
      uint pad2;
};

struct IndexedIndirectCommand
{
    // CommandSignature:
    // 0: ROOT_CONSTANT b2, 1 uint
    // 1: DRAW_INDEXED
      uint baseInstance;

      uint indexCountPerInstance;
      uint instanceCount;
      uint startIndexLocation;
      int baseVertexLocation;
      uint startInstanceLocation;
};

cbuffer Params : register(b0)
{
      uint gBatchCount;
      uint3 pad;
};

StructuredBuffer<BatchInfo> gBatches : register(t0);
RWStructuredBuffer<IndexedIndirectCommand> gCommands : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
      uint batchIndex = id.x;
      if (batchIndex >= gBatchCount)
      {
            return;
      }

      BatchInfo batch = gBatches[batchIndex];

      IndexedIndirectCommand cmd;
      cmd.baseInstance = batch.baseInstance;

      cmd.indexCountPerInstance = batch.indexCountPerInstance;
      cmd.instanceCount = batch.instanceCount;
      cmd.startIndexLocation = batch.startIndexLocation;
      cmd.baseVertexLocation = batch.baseVertexLocation;

    // 今のVSは RootConstant の baseInstance を足して読むので、
    // Draw側の StartInstanceLocation は 0 にしておく。
      cmd.startInstanceLocation = 0;

      gCommands[batchIndex] = cmd;
}