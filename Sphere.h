#pragma once
#include "Model.h"

// 前方宣言
struct VertexData;
class TUFEngine;

class Sphere : public Model {
public:
    Sphere() = default;
    ~Sphere() override;

    // ⭕ 引数をエンジンだけに修正して、自己完結させます
    void InitSphere(TUFEngine* engine);

    void Update();
    void SetWorldTransform(const Matrix4x4& wvp, const Matrix4x4& world) override;
    void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex) override;
    void SetLightResource(ID3D12Resource* lightResource);
    void UpdateVertices(const Vector3& points, const Vector2& texcoord, const Vector3& normal, int index) override {}

private:
    ID3D12Resource* m_pVertexResource = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};

    ID3D12Resource* m_pIndexResource = nullptr;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};

    ID3D12Resource* m_pMaterialResource = nullptr;
    ID3D12Resource* m_pWvpResource = nullptr;
    ID3D12Resource* m_pLightResource = nullptr;
    bool m_ownsLightResource = false;

    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;

    TUFEngine* m_pEngine = nullptr;

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }

};
