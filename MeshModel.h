#pragma once
#include "Model.h"
#include <wrl.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using Microsoft::WRL::ComPtr;   // ★追加

struct VertexData;
class TUFEngine;

struct MaterialData
{
    std::string textureFilPath;
    std::string normalTextureFilePath;
};

struct ModelData
{
    std::vector<VertexData> vertices;
    MaterialData material;
};

class MeshModel : public Model
{
public:
    MeshModel();
    ~MeshModel();

    bool LoadFromOBJ(
        const std::string& directoryPath,
        const std::string& filename);

    void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex) override;
    void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex, UINT startInstanceLocation);

    MaterialData LoadMaterialTemplateFile(
        const std::string& directoryPath,
        const std::string& filename);

    void InitMeshModel(TUFEngine* engine);

    void SetTextureIndex(int index) { m_textureIndex = index; }
    int  GetTextureIndex() const { return m_textureIndex; }

    void SetNormalMapIndex(int idx) { m_normalTextureIndex = idx; }
    int  GetNormalMapIndex() const { return m_normalTextureIndex; }

    void UpdateVertices(
        const Vector3& points,
        const Vector2& texcoord,
        const Vector3& normal,
        int index) override;

private:
    ModelData            modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string          line;
    ComPtr<ID3D12Resource> m_pMaterialResource;
    ComPtr<ID3D12Resource>   m_vertexBuffer;      // 元々 ComPtr のため実質変更なし
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
    size_t                   m_vertexCount = 0;
    TUFEngine* m_pEngine = nullptr;

    int m_textureIndex = -1;
    int m_normalTextureIndex = -1;

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }
};
