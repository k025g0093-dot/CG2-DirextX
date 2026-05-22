#pragma once
#include "Model.h"
#include <wrl.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;   // ★追加

struct VertexData;
class TUFEngine;

class TriangleModel : public Model {
public:
    void Initialize(TUFEngine* engine);

    void UpdateVertices(const Vector3& points,
        const Vector2& texcoord,
        const Vector3& normal,
        int index) override;

    void Draw(ID3D12GraphicsCommandList* cmdList, int index) override;

private:
    ComPtr<ID3D12Resource>       m_pVertexResource;   // ★
    D3D12_VERTEX_BUFFER_VIEW     m_vertexBufferView{};
    ComPtr<ID3D12Resource>       m_pMaterialResource; // ★

    VertexData* m_pVertexData = nullptr;

    uint32_t    m_vertexCount = 0;
    TUFEngine* m_pEngine = nullptr;

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }
};