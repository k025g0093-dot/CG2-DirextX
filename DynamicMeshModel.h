#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DynamicMesh.h"

using Microsoft::WRL::ComPtr;

class TUFEngine;
struct VertexData;
struct Material; // 🌟追加：C++側のマテリアル構造体の前方宣言
struct Vector3;  // 🌟追加：UVのスケールや移動に使うベクトル構造体の前方宣言

class DynamicMeshModel {
public:
    bool Init(TUFEngine* engine, int gridW, int gridH);
    void UpdateHeights(const DynamicMesh& mesh);
    void Draw(ID3D12GraphicsCommandList* cmdList,
        int textureIndex,
        UINT instanceCount,
        UINT startInstanceLocation);
    // 🌟追加：外部からUVのタイリング・回転・移動量を指定して行列を更新する関数
    void UpdateUVTransform(const Vector3& uvScale, float uvRotation, const Vector3& uvTranslation);

private:
    ComPtr<ID3D12Resource>   m_vertexBuffer;
    ComPtr<ID3D12Resource>   m_indexBuffer;
    ComPtr<ID3D12Resource>   m_materialBuffer;
    ComPtr<ID3D12Resource>   m_lightBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    D3D12_INDEX_BUFFER_VIEW  m_indexBufferView{};

    VertexData* m_mappedData = nullptr;
    uint32_t* m_mappedIndex = nullptr;
    Material* m_mappedMaterial = nullptr;       // 🌟追加：Mapしたマテリアルバッファのポインタを保持する

    uint32_t     m_vertexCount = 0;                // 実頂点数 W*H
    uint32_t     m_indexCount = 0;                // インデックス数 (W-1)*(H-1)*6
    int          m_gridW = 0;
    int          m_gridH = 0;
};