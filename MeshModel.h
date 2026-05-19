#pragma once
#include "Model.h"
#include <wrl.h>
#include <vector>
#include <string>

#include <fstream>
#include <sstream>

//前方宣言
struct VertexData;
class TUFEngine;
struct ModelData
{
	std::vector<VertexData> vertices;
};

class MeshModel :public Model
{
public:


	MeshModel();
	~MeshModel();

	bool LoadFromOBJ(
		const std::string& directoryPath,
		const std::string& filename
	);

	void Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex) override;

	void InitMeshModel(TUFEngine* engine);
	void SetTextureIndex(int index) { m_textureIndex = index; }
	int GetTextureIndex() const { return m_textureIndex; }

	void UpdateVertices(
		const Vector3& points,
		const Vector2& texcoord,
		const Vector3& normal,
		int index) override;
private:
	ModelData modelData;
	std::vector<Vector4>positions;//位置
	std::vector<Vector3>normals;//法線
	std::vector<Vector2>texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ一行を記録
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
	size_t m_vertexCount = 0;
	TUFEngine* m_pEngine = nullptr;
	int m_textureIndex = 0;
};

