#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DynamicMesh.h"

using Microsoft::WRL::ComPtr;

class TUFEngine;
struct VertexData;
struct Material;
struct Vector3;

class DynamicMeshModel {
public:
    bool Init(TUFEngine* engine, int gridW, int gridH);
    void UpdateHeights(const DynamicMesh& mesh);
    void Draw(ID3D12GraphicsCommandList* cmdList,
        int textureIndex,
        UINT instanceCount,
        UINT startInstanceLocation);
    void UpdateUVTransform(const Vector3& uvScale, float uvRotation, const Vector3& uvTranslation);

    //新規追加：フレーム終了時にバッファを切り替え
    void SwapBuffers() {
        m_currentBufferIndex = (m_currentBufferIndex + 1) % 2;
    }

private:
    ComPtr<ID3D12Resource>   m_vertexBuffers[2];
    ComPtr<ID3D12Resource>   m_indexBuffer;
    ComPtr<ID3D12Resource>   m_materialBuffer;
    ComPtr<ID3D12Resource>   m_lightBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferViews[2]; 
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView;

    VertexData* m_mappedDatas[2] = { nullptr, nullptr };  
    uint32_t* m_mappedIndex = nullptr;
    Material* m_mappedMaterial = nullptr;

    //バッファインデックス（毎フレーム切り替え）
    int m_currentBufferIndex = 0;  // 書き込み対象：0 or 1

    uint32_t     m_vertexCount = 0;
    uint32_t     m_indexCount = 0;
    int          m_gridW = 0;
    int          m_gridH = 0;
};