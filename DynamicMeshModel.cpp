#include "DynamicMeshModel.h"
#include "TextureManager.h"
#include "TUFEngine.h"

bool DynamicMeshModel::Init(TUFEngine* engine, int gridW, int gridH) {
    m_vertexCount = (uint32_t)((gridW - 1) * (gridH - 1) * 6);

    m_vertexBuffer = CreateBufferResource(
        engine->GetDevice(), sizeof(VertexData) * m_vertexCount);
    if (!m_vertexBuffer) return false;

    m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData));
    memset(m_mappedData, 0, sizeof(VertexData) * m_vertexCount);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(VertexData) * m_vertexCount;
    m_vertexBufferView.StrideInBytes = sizeof(VertexData);

    m_materialBuffer = CreateBufferResource(engine->GetDevice(), sizeof(Material));
    Material* materialData = nullptr;
    m_materialBuffer->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->enableLifhting = true;
    materialData->uvTransform = MakeIdentity4x4();
    m_materialBuffer->Unmap(0, nullptr);

    m_lightBuffer = CreateBufferResource(engine->GetDevice(), sizeof(DirectionalLLight));
    DirectionalLLight* lightData = nullptr;
    m_lightBuffer->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
    lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightData->direction = { 0.0f, -1.0f, 0.0f };
    lightData->intensity = 1.0f;
    m_lightBuffer->Unmap(0, nullptr);

    return true;
}

void DynamicMeshModel::SyncFrom(const DynamicMesh& mesh) {
    if (!m_mappedData) return;

    auto& verts = mesh.getVertices();
    auto& normals = mesh.getNormals();
    auto& indices = mesh.getIndices();

    int vi = 0;
    for (int i = 0; i < (int)indices.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            int idx = indices[i + j] * 3;
            m_mappedData[vi].position = { verts[idx], verts[idx + 1], verts[idx + 2], 1.0f };
            m_mappedData[vi].normal = { normals[idx], normals[idx + 1], normals[idx + 2] };
            m_mappedData[vi].texcoord = {
                (verts[idx] + mesh.getGridW() / 2.0f) / (float)mesh.getGridW(),
                (verts[idx + 2] + mesh.getGridH() / 2.0f) / (float)mesh.getGridH()
            };
            vi++;
        }
    }
}

void DynamicMeshModel::Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex) {
    if (m_vertexCount == 0 || !m_vertexBuffer) return;

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto handle = TextureManager::GetInstance()->GetGPUHandle(textureIndex);
    if (handle.ptr == 0) handle = TextureManager::GetInstance()->GetGPUHandle(0);
    if (handle.ptr == 0) return;

    if (m_materialBuffer) {
        cmdList->SetGraphicsRootConstantBufferView(0, m_materialBuffer->GetGPUVirtualAddress());
    }
    if (m_lightBuffer) {
        cmdList->SetGraphicsRootConstantBufferView(3, m_lightBuffer->GetGPUVirtualAddress());
    }
    cmdList->SetGraphicsRootDescriptorTable(2, handle);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    cmdList->DrawInstanced(m_vertexCount, 1, 0, 0);
}
