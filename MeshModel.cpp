#include "MeshModel.h"
#include "TUFEngine.h"

extern ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

MeshModel::MeshModel() = default;
MeshModel::~MeshModel() = default;

void MeshModel::InitMeshModel(TUFEngine* engine) {
	m_pEngine = engine;
}

bool MeshModel::LoadFromOBJ(
	const std::string& directoryPath,
	const std::string& filename
) {
	ID3D12Device* device = m_pEngine->GetDevice();
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // 開けない時はやめる

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を確認

		if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt")
		{
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f")
		{
			VertexData traiangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/'); // /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				// ⭕ 左手系変換：位置と法線のX軸を反転
				position.x *= -1.0f;
				normal.x *= -1.0f;

				traiangle[faceVertex] = { position, texcoord, normal };
			}

			// ⭕ X反転によって自動的に時計回りになるので、結び順はひっくり返さず【そのまま】入れる！
			modelData.vertices.push_back(traiangle[2]);
			modelData.vertices.push_back(traiangle[1]);
			modelData.vertices.push_back(traiangle[0]);
		}
		else if (identifier == "mtllib")
		{
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		
			if (!modelData.material.textureFilPath.empty())
			{
				// TextureManagerにロードを依頼し、インデックスを受け取る
				int textureIndex = TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilPath);

				// 読み込んだインデックスを自分自身に保存する
				SetTextureIndex(textureIndex);
			}
		}
	}

	m_vertexCount = static_cast<uint32_t>(modelData.vertices.size());
	if (m_vertexCount > 0) {

		m_vertexBuffer = CreateBufferResource(device, sizeof(VertexData) * m_vertexCount);

		if (!m_vertexBuffer) {
			OutputDebugStringA("Error: CreateBufferResource failed!\n");
			return false;
		}

		void* pData = nullptr;
		HRESULT hr = m_vertexBuffer->Map(0, nullptr, &pData);
		if (SUCCEEDED(hr)) {
			std::memcpy(pData, modelData.vertices.data(), sizeof(VertexData) * m_vertexCount);
			m_vertexBuffer->Unmap(0, nullptr);
		}
		else {
			OutputDebugStringA("Error: VertexBuffer Map failed!\n");
			return false;
		}

		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = (UINT)sizeof(VertexData) * (UINT)m_vertexCount;
		m_vertexBufferView.StrideInBytes = sizeof(VertexData);
	}

	return true;
}


MaterialData MeshModel::LoadMaterialTemplateFile(
	const std::string& directoryPath,
	const std::string& filename
) {

	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + '/' + filename);//ファイルを開く
	assert(file.is_open());//開けない場合はやめる

	while (std::getline(file, line))
	{
		std::string identifire;
		std::istringstream s(line);
		s >> identifire;

		if (identifire == "map_kd")
		{
			//identifireに応じた処理
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilPath = directoryPath + "/" + textureFilename;

		}

	}

	return materialData;

}

void MeshModel::Draw(ID3D12GraphicsCommandList* cmdList, int textureIndex) {
	if (m_vertexCount == 0 || !m_vertexBuffer) return;

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto handle = TextureManager::GetInstance()->GetGPUHandle(textureIndex);
	if (handle.ptr == 0) {
		handle = TextureManager::GetInstance()->GetGPUHandle(0);
		if (handle.ptr == 0) return;
	}
	cmdList->SetGraphicsRootDescriptorTable(2, handle);
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmdList->DrawInstanced(static_cast<UINT>(m_vertexCount), 1, 0, 0);
}

// ⭕ 重複を排除し、X反転を正しく行うように一本化した UpdateVertices
void MeshModel::UpdateVertices(
	const Vector3& points,
	const Vector2& texcoord,
	const Vector3& normal,
	int index)
{
	if (!m_vertexBuffer || index < 0 || static_cast<uint32_t>(index) >= m_vertexCount) return;

	void* pData = nullptr;
	m_vertexBuffer->Map(0, nullptr, &pData);

	VertexData* vertices = static_cast<VertexData*>(pData);

	// ⭕ LoadFromOBJと同じく、動的更新時もX軸を反転させて上書きする
	vertices[index].position = { points.x * -1.0f, points.y, points.z, 1.0f };
	vertices[index].texcoord = texcoord;
	vertices[index].normal = { normal.x * -1.0f, normal.y, normal.z };

	m_vertexBuffer->Unmap(0, nullptr);
}