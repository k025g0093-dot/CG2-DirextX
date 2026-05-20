#pragma once
#include "Model.h"
#include <wrl.h>
#include <d3d12.h>

// 前方宣言
struct VertexData;
class TUFEngine;

class TriangleModel : public Model {
public:
    void Initialize(TUFEngine* engine);

    // 基底クラス(Model)の関数をオーバーライド
    void UpdateVertices(const Vector3& points,
        const Vector2& texcoord,
        const Vector3& normal,
        int index) override;

    void Draw(ID3D12GraphicsCommandList* cmdList, int index) override;

private:
    ID3D12Resource* m_pVertexResource = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    ID3D12Resource* m_pMaterialResource = nullptr;

    // ⭕ 初期化時に1回だけMapしたポインタを保持する変数
    VertexData* m_pVertexData = nullptr;

    uint32_t m_vertexCount = 0;
    TUFEngine* m_pEngine = nullptr;

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }
};