#pragma once
#include "allVector.h" 
#include <d3d12.h>
#include <vector>

struct Vertex {
    Vector4 position;  
    Vector2 texcoord;  
    Vector3 normal; 
    Vector3 tangent;
};

class Model {
protected:
    // 変換パラメータ（自作のVector3を使用）
    Vector3 m_position = { 0, 0, 0 };
    Vector3 m_rotation = { 0, 0, 0 };
    Vector3 m_scale = { 1, 1, 1 };

    Vertex* m_pVertexData = nullptr; // 頂点バッファへのマップポインタ
    UINT m_indexCount = 0;
public:
    // Model.h など
public:
    // 中身は書かず、「= 0」にして子クラスに実装を強制する
    virtual UINT GetIndexCount() const { return m_indexCount; }
    virtual ~Model() {}

    // 変換パラメータの設定
    void SetPosition(const Vector3& p) { m_position = p; }
    void SetRotation(const Vector3& r) { m_rotation = r; }
    void SetScale(const Vector3& s) { m_scale = s; }

    // ワールド行列の計算（自作の Matrix4x4 を返す形に戻す）
    Matrix4x4 GetWorldMatrix() const;
    virtual void SetWorldTransform(const Matrix4x4&, const Matrix4x4&) {}
    virtual void SetUVTransform(const Matrix4x4&) {}

    // 頂点データの更新（子クラスで実装。型を自作のものへ変更）
    virtual void UpdateVertices(
        const Vector3& points,
        const Vector2& texcoord,
        const Vector3& normal,
        int index) = 0;

    // 描画関数（引数にテクスチャインデックスを受け取れる形を維持）
    virtual void Draw(
        ID3D12GraphicsCommandList* cmdList,
        int textureIndex,
        UINT instanceCount,
        UINT startInstanceLocation) = 0;

};
