#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DynamicMesh.h"

using Microsoft::WRL::ComPtr;

class TUFEngine;
struct VertexData;

class DynamicMeshModel {
public:
    bool Init(TUFEngine* engine, int gridW, int gridH);
    void SyncFrom(const DynamicMesh& mesh);
    void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex);

private:
    ComPtr<ID3D12Resource>   m_vertexBuffer;
    ComPtr<ID3D12Resource>   m_indexBuffer;       // ★追加
    ComPtr<ID3D12Resource>   m_materialBuffer;
    ComPtr<ID3D12Resource>   m_lightBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView{};  // ★追加
    VertexData* m_mappedData = nullptr;
    uint32_t* m_mappedIndex = nullptr;          // ★追加
    uint32_t     m_vertexCount = 0;                // 実頂点数 W*H
    uint32_t     m_indexCount = 0;                // インデックス数 (W-1)*(H-1)*6
    int          m_gridW = 0;
    int          m_gridH = 0;
};