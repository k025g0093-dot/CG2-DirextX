#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DynamicMesh.h"
class TUFEngine;
struct VertexData;
class DynamicMeshModel {
public:
    bool Init(TUFEngine* engine, int gridW, int gridH);
    void SyncFrom(const DynamicMesh& mesh);
    void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW               m_vertexBufferView{};
    VertexData* m_mappedData = nullptr;
    uint32_t                               m_vertexCount = 0;
};