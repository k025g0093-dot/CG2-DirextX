#include "DynamicMeshModel.h"
#include "TextureManager.h"
#include "TUFEngine.h"

bool DynamicMeshModel::Init(TUFEngine* engine, int gridW, int gridH) {
    m_gridW = gridW;
    m_gridH = gridH;

    // 実頂点数は W*H
    m_vertexCount = (uint32_t)(gridW * gridH);
    m_indexCount = (uint32_t)((gridW - 1) * (gridH - 1) * 6);

    // -------------------------------------------------------
    // 頂点バッファ（毎フレーム書き換える → Map したまま）
    // -------------------------------------------------------
    m_vertexBuffer = CreateBufferResource(
        engine->GetDevice(), sizeof(VertexData) * m_vertexCount);
    if (!m_vertexBuffer) return false;
    m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData));
    memset(m_mappedData, 0, sizeof(VertexData) * m_vertexCount);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(VertexData) * m_vertexCount;
    m_vertexBufferView.StrideInBytes = sizeof(VertexData);

    // -------------------------------------------------------
    // インデックスバッファ（初回のみ書いてあとは触らない）
    // -------------------------------------------------------
    m_indexBuffer = CreateBufferResource(
        engine->GetDevice(), sizeof(uint32_t) * m_indexCount);
    if (!m_indexBuffer) return false;
    m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedIndex));

    int ii = 0;
    for (int y = 0; y < gridH - 1; y++) {
        for (int x = 0; x < gridW - 1; x++) {
            uint32_t tl = y * gridW + x;
            uint32_t tr = y * gridW + (x + 1);
            uint32_t bl = (y + 1) * gridW + x;
            uint32_t br = (y + 1) * gridW + (x + 1);
            // 三角形①
            m_mappedIndex[ii++] = tl;
            m_mappedIndex[ii++] = bl;
            m_mappedIndex[ii++] = tr;
            // 三角形②
            m_mappedIndex[ii++] = tr;
            m_mappedIndex[ii++] = bl;
            m_mappedIndex[ii++] = br;
        }
    }
    m_indexBuffer->Unmap(0, nullptr);
    m_mappedIndex = nullptr;

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = sizeof(uint32_t) * m_indexCount;
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // -------------------------------------------------------
    // ★ マテリアルバッファ（動的更新のため Unmap せずに保持する）
    // -------------------------------------------------------
    m_materialBuffer = CreateBufferResource(engine->GetDevice(), sizeof(Material));
    if (!m_materialBuffer) return false;

    // 🌟頂点バッファと同様に、メンバ変数（m_mappedMaterial）にアドレスを固定する
    m_materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedMaterial));

    // 初期値の設定
    m_mappedMaterial->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_mappedMaterial->enableLighting = 1; // シェーダーのint型に合わせる
    m_mappedMaterial->enableNormalMap = 0;
    m_mappedMaterial->uvTransform = MakeIdentity4x4(); // 初期状態は単位行列

    // -------------------------------------------------------
    // ライトバッファ（元のまま）
    // -------------------------------------------------------
    m_lightBuffer = CreateBufferResource(engine->GetDevice(), sizeof(DirectionalLight));
    if (!m_lightBuffer) return false;
    DirectionalLight* lightData = nullptr;
    m_lightBuffer->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
    lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightData->direction = { 0.0f, -1.0f, 0.0f };
    lightData->intensity = 1.0f;
    m_lightBuffer->Unmap(0, nullptr);

    return true;
}

// 🌟新設：外部（ImGuiやUpdate処理）からUVのタイリング・回転・移動を変更するための関数
void DynamicMeshModel::UpdateUVTransform(const Vector3& uvScale, float uvRotation, const Vector3& uvTranslation) {
    if (!m_mappedMaterial) return;

    // 以前ギズモでも使用した MakeAffineMatrix を使って、UV用のトランスフォーム行列を計算
    // ※UVは2次元なので、回転はZ軸（3番目の引数）に適用します
    m_mappedMaterial->uvTransform = MakeAffineMatrix(uvScale, { 0.0f, 0.0f, uvRotation }, uvTranslation);
}

void DynamicMeshModel::SyncFrom(const DynamicMesh& mesh) {
    if (!m_mappedData) return;

    auto& verts = mesh.getVertices();
    auto& normals = mesh.getNormals();

    // インデックス展開なし・W*H 頂点をそのまま書くだけ
    for (int i = 0; i < (int)m_vertexCount; i++) {
        int idx = i * 3;
        m_mappedData[i].position = { verts[idx], verts[idx + 1], verts[idx + 2], 1.0f };
        m_mappedData[i].normal = { normals[idx], normals[idx + 1], normals[idx + 2] };
        m_mappedData[i].texcoord = {
            (verts[idx] + m_gridW / 2.0f) / (float)m_gridW,
            (verts[idx + 2] + m_gridH / 2.0f) / (float)m_gridH
        };
        m_mappedData[i].tangent = { 1.0f, 0.0f, 0.0f };
    }
}

void DynamicMeshModel::Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex) {
    if (m_indexCount == 0 || !m_vertexBuffer || !m_indexBuffer) return;

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto handle = TextureManager::GetInstance()->GetGPUHandle(textureIndex);
    if (handle.ptr == 0) handle = TextureManager::GetInstance()->GetGPUHandle(0);
    if (handle.ptr == 0) return;

    if (m_materialBuffer)
        cmdList->SetGraphicsRootConstantBufferView(0, m_materialBuffer->GetGPUVirtualAddress());
    if (m_lightBuffer)
        cmdList->SetGraphicsRootConstantBufferView(3, m_lightBuffer->GetGPUVirtualAddress());

    cmdList->SetGraphicsRootDescriptorTable(2, handle);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // インデックスバッファをセットして DrawIndexedInstanced で描画
    cmdList->IASetIndexBuffer(&m_indexBufferView);
    cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}