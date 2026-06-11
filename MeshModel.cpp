#include "MeshModel.h"
#include "TUFEngine.h"

MeshModel::MeshModel() = default;
MeshModel::~MeshModel() = default;

void MeshModel::InitMeshModel(TUFEngine* engine) {
	m_pEngine = engine;
	ID3D12Device* device = engine->GetDevice();

	m_pMaterialResource = CreateBufferResource(device, Align256(sizeof(Material)));
	Material* materialData = nullptr;
	m_pMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 0.5f, 0.5f, 0.5f, 0.5f };
	materialData->enableLighting = 1;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->enableNormalMap = 0;
	m_pMaterialResource->Unmap(0, nullptr);
}

bool MeshModel::LoadFromOBJ(
	const std::string& directoryPath,
	const std::string& filename
) {
	ID3D12Device* device = m_pEngine->GetDevice();
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	std::vector<VertexData> faceVertices;

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

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
			std::vector<VertexData> currentFaceVertices;
			std::string vertexDefinition;

			while (s >> vertexDefinition) {
				std::istringstream vStream(vertexDefinition);
				uint32_t indices[3] = { 0, 0, 0 };

				for (int32_t i = 0; i < 3; ++i) {
					std::string indexStr;
					if (!std::getline(vStream, indexStr, '/')) break;
					if (!indexStr.empty()) {
						indices[i] = std::stoi(indexStr);
					}
				}

				if (indices[0] == 0) continue;

				uint32_t pIdx = indices[0] - 1;
				uint32_t tIdx = (indices[1] > 0 && indices[1] <= texcoords.size()) ? indices[1] - 1 : 0;
				uint32_t nIdx = (indices[2] > 0 && indices[2] <= normals.size()) ? indices[2] - 1 : 0;

				Vector4 pos = positions[pIdx];
				Vector2 uv = (texcoords.size() > 0) ? texcoords[tIdx] : Vector2{ 0,0 };
				Vector3 norm = (normals.size() > 0) ? normals[nIdx] : Vector3{ 0,0,0 };

				pos.x *= -1.0f;
				norm.x *= -1.0f;

				currentFaceVertices.push_back({ pos, uv, norm });
			}

			for (size_t i = 2; i < currentFaceVertices.size(); ++i) {
				modelData.vertices.push_back(currentFaceVertices[i]);
				modelData.vertices.push_back(currentFaceVertices[i - 1]);
				modelData.vertices.push_back(currentFaceVertices[0]);
			}
		}
		else if (identifier == "mtllib")
		{
			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);

			if (!modelData.material.textureFilPath.empty())
			{
				int textureIndex = TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilPath);
				SetTextureIndex(textureIndex);
			}

			if (!modelData.material.normalTextureFilePath.empty()) {
				int normalIndex = TextureManager::GetInstance()->LoadTexture(modelData.material.normalTextureFilePath);
				m_normalTextureIndex = normalIndex;
			}
		}
	}

	m_vertexCount = static_cast<uint32_t>(modelData.vertices.size());
	if (m_vertexCount > 0) {

		// 頂点バッファ作成
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

		// ★ インデックスバッファ作成
		// OBJ は LoadFromOBJ 内で既に三角形分割済みなので
		// シーケンシャルなインデックス (0, 1, 2, 3, ...) で十分
		m_indexCount = (uint32_t)m_vertexCount;
		m_indexBuffer = CreateBufferResource(device, sizeof(uint32_t) * m_indexCount);

		if (!m_indexBuffer) {
			OutputDebugStringA("Error: IndexBuffer CreateBufferResource failed!\n");
			return false;
		}

		uint32_t* indexData = nullptr;
		hr = m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		if (SUCCEEDED(hr)) {
			for (uint32_t i = 0; i < m_indexCount; i++) {
				indexData[i] = i;
			}
			m_indexBuffer->Unmap(0, nullptr);
		}
		else {
			OutputDebugStringA("Error: IndexBuffer Map failed!\n");
			return false;
		}

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = sizeof(uint32_t) * m_indexCount;
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	}

	return true;
}


MaterialData MeshModel::LoadMaterialTemplateFile(
	const std::string& directoryPath,
	const std::string& filename
) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifire;
		std::istringstream s(line);
		s >> identifire;

		if (identifire == "map_Kd")
		{
			std::string token;
			std::string textureFilename;
			while (s >> token) {
				if (token[0] == '-') {
					if (token == "-s" || token == "-o") {
						float tmp; s >> tmp >> tmp >> tmp;
					}
					else if (token == "-bm") {
						float tmp; s >> tmp;
					}
					continue;
				}
				textureFilename = token;
			}

			std::replace(textureFilename.begin(), textureFilename.end(), '\\', '/');

			if (!textureFilename.empty()) {
				materialData.textureFilPath = directoryPath + "/" + textureFilename;
			}
		}
		else if (identifire == "map_Bump" || identifire == "bump") {
			std::string token;
			std::string textureFilename;
			while (s >> token) {
				if (token[0] == '-') {
					if (token == "-s" || token == "-o") {
						float tmp; s >> tmp >> tmp >> tmp;
					}
					else if (token == "-bm") {
						float tmp; s >> tmp;
					}
					continue;
				}
				textureFilename = token;
			}

			std::replace(textureFilename.begin(), textureFilename.end(), '\\', '/');

			if (!textureFilename.empty()) {
				materialData.normalTextureFilePath = directoryPath + "/" + textureFilename;
			}
		}
	}

	return materialData;
}


void MeshModel::Draw(
	ID3D12GraphicsCommandList* cmdList,
	int textureIndex,
	UINT instanceCount,
	UINT startInstanceLocation
)
{
	if (m_vertexCount == 0 || !m_vertexBuffer || !m_indexBuffer) return;

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetGraphicsRootConstantBufferView(0, m_pMaterialResource->GetGPUVirtualAddress());

	bool hasNormalTexture = false;

	if (textureIndex >= 0) {
		auto handle = TextureManager::GetInstance()->GetGPUHandle(textureIndex);
		if (handle.ptr != 0) {
			cmdList->SetGraphicsRootDescriptorTable(2, handle);
		}

		if (m_normalTextureIndex >= 0) {
			auto normalHandle = TextureManager::GetInstance()->GetGPUHandle(m_normalTextureIndex);
			if (normalHandle.ptr != 0) {
				cmdList->SetGraphicsRootDescriptorTable(4, normalHandle);
				hasNormalTexture = true;
			}
		}
	}
	else {
		Material* materialData = nullptr;
		m_pMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		materialData->enableLighting = -1;
		materialData->enableNormalMap = 0;
		m_pMaterialResource->Unmap(0, nullptr);
	}

	if (textureIndex >= 0) {
		Material* materialData = nullptr;
		m_pMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		materialData->enableNormalMap = hasNormalTexture ? 1 : 0;
		m_pMaterialResource->Unmap(0, nullptr);
	}

	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmdList->IASetIndexBuffer(&m_indexBufferView);  // ★ 追加

	// ★ DrawInstanced → DrawIndexedInstanced に変更
	// Sphere と描画命令を統一することでインスタンスバッファのズレをなくす
	cmdList->DrawIndexedInstanced(
		m_indexCount,
		instanceCount,
		0,
		0,
		startInstanceLocation
	);
}

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

	vertices[index].position = { points.x * -1.0f, points.y, points.z, 1.0f };
	vertices[index].texcoord = texcoord;
	vertices[index].normal = { normal.x * -1.0f, normal.y, normal.z };
	vertices[index].tangent = { 0.0f, 0.0f, 0.0f };

	m_vertexBuffer->Unmap(0, nullptr);
}