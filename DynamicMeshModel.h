#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DynamicMesh.h"

using Microsoft::WRL::ComPtr;   // ★追加（元々 wrl.h は include 済みだったので using のみ追加）

class TUFEngine;
struct VertexData;

class DynamicMeshModel {
public:
    bool Init(TUFEngine* engine, int gridW, int gridH);
    void SyncFrom(const DynamicMesh& mesh);
    void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex);

private:
    ComPtr<ID3D12Resource>   m_vertexBuffer;      // ★ 元々 ComPtr だったため変更なし
    ComPtr<ID3D12Resource>   m_materialBuffer;
    ComPtr<ID3D12Resource>   m_lightBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    VertexData* m_mappedData = nullptr;
    uint32_t                 m_vertexCount = 0;
};
