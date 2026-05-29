#include "TriangleModel.h"
#include <wrl.h>
#include "TUFEngine.h"

void TriangleModel::Initialize(TUFEngine* engine) {
    m_pEngine = engine;
    ID3D12Device* device = engine->GetDevice();

    const UINT maxVertices = 3 * 10000;
    const UINT bufferSize = sizeof(VertexData) * maxVertices;
    m_vertexCount = maxVertices;

    m_pVertexResource = CreateBufferResource(device, bufferSize);
    m_pVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&m_pVertexData));

    m_vertexBufferView.BufferLocation = m_pVertexResource->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(VertexData);
    m_vertexBufferView.SizeInBytes = bufferSize;

    m_pWvpResource = CreateBufferResource(device, Align256(sizeof(TransformationMatrix)));
    TransformationMatrix* wvpData = nullptr;
    m_pWvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
    wvpData->WVP = MakeIdentity4x4();
    wvpData->World = MakeIdentity4x4();
    m_pWvpResource->Unmap(0, nullptr);


    // ★ Mapしたまま保持してDrawのたびに色を書き換えられるようにする
    m_pMaterialResource = CreateBufferResource(device, Align256(sizeof(Material)));
    m_pMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->enableLifhting = false;
    materialData->uvTransform = MakeIdentity4x4();

    m_pLightResource = CreateBufferResource(device, Align256(sizeof(DirectionalLLight)));
    DirectionalLLight* lightData = nullptr;
    m_pLightResource->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
    lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightData->direction = { 0.0f, -1.0f, 0.0f };
    lightData->intensity = 1.0f;
    m_pLightResource->Unmap(0, nullptr);
}

void TriangleModel::Update(const Vector3& pos, const Vector3& rot, const Vector3& scale) {
    Matrix4x4 viewProjectionMatrix = m_pEngine->GetViewProjectionMatrix();

    Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
    Matrix4x4 rotateX = MakeRotateXMatrix(rot.x);
    Matrix4x4 rotateY = MakeRotateYMatrix(rot.y);
    Matrix4x4 rotateZ = MakeRotateZMatrix(rot.z);
    Matrix4x4 translateMatrix = MakeTranslateMatrix(pos);

    Matrix4x4 worldMatrix = Multiply(scaleMatrix,
        Multiply(rotateX,
            Multiply(rotateY,
                Multiply(rotateZ, translateMatrix))));

    Matrix4x4 wvpMatrix = Multiply(worldMatrix, viewProjectionMatrix);
    SetWorldTransform(wvpMatrix, worldMatrix);
}

void TriangleModel::UpdateVertices(const Vector3& points,
    const Vector2& texcoord,
    const Vector3& normal,
    int index)
{
    if (!m_pVertexData || index < 0 || static_cast<uint32_t>(index) >= m_vertexCount) return;

    m_pVertexData[index].position = { points.x * -1.0f, points.y, points.z, 1.0f };
    m_pVertexData[index].texcoord = texcoord;
    m_pVertexData[index].normal = { normal.x * -1.0f, normal.y, normal.z };
}

void TriangleModel::SetWorldTransform(const Matrix4x4& wvp, const Matrix4x4& world) {
    TransformationMatrix* wvpData = nullptr;
    m_pWvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
    wvpData->WVP = wvp;
    wvpData->World = world;
    m_pWvpResource->Unmap(0, nullptr);
}

void TriangleModel::Draw(ID3D12GraphicsCommandList* cmdList, int index) {
    Draw(cmdList, index, 0);
}

void TriangleModel::Draw(ID3D12GraphicsCommandList* cmdList, int drawIndex, int textureIndex, const Vector4& color) {
    if (m_vertexCount == 0 || !m_pVertexResource) return;

    // ★ 毎フレーム色を更新
    if (materialData) {
        materialData->color = color;
    }

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    cmdList->SetGraphicsRootConstantBufferView(0, m_pMaterialResource->GetGPUVirtualAddress());

    if (textureIndex >= 0) {
        cmdList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(textureIndex));
    }
    if (m_pLightResource) {
        cmdList->SetGraphicsRootConstantBufferView(3, m_pLightResource->GetGPUVirtualAddress());
    }

    cmdList->DrawInstanced(3, 1, drawIndex * 3, 0);
}