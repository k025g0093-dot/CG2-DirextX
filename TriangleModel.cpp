#include "TriangleModel.h"
#include <wrl.h>
#include "TriangleModel.h"
#include "TUFEngine.h"

// 外部のバッファ作成関数
extern ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

void TriangleModel::Initialize(TUFEngine* engine) {
	m_pEngine = engine;
	ID3D12Device* device = engine->GetDevice();

	// 最大30,000頂点（10,000枚の三角形）分のバッファを確保
	const UINT maxVertices = 3 * 10000;
	const UINT bufferSize = sizeof(VertexData) * maxVertices;

	m_vertexCount = maxVertices;

	// 頂点リソースの作成
	m_pVertexResource = CreateBufferResource(device, bufferSize);

	// ⭕ 理想のコードと同じ：初期化時に1回だけMapしてポインタをずっと保持する（Unmapしない）
	m_pVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&m_pVertexData));

	// ビューの設定
	m_vertexBufferView.BufferLocation = m_pVertexResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(VertexData);
	m_vertexBufferView.SizeInBytes = bufferSize;

	m_pMaterialResource = CreateBufferResource(device, Align256(sizeof(Material)));
	Material* materialData = nullptr;
	m_pMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLifhting = true;//ここは要修正
	materialData->uvTransform = MakeIdentity4x4();
	m_pMaterialResource->Unmap(0, nullptr);
}

void TriangleModel::UpdateVertices(const Vector3& points,
	const Vector2& texcoord,
	const Vector3& normal,
	int index)
{
	// ⭕ 常時Mapされているポインタ（m_pVertexData）に直接書き込む
	if (!m_pVertexData || index < 0 || static_cast<uint32_t>(index) >= m_vertexCount) return;

	// メッシュモデルと同じ変換（X軸反転など）をかけて安全に代入
	m_pVertexData[index].position = { points.x * -1.0f, points.y, points.z, 1.0f };
	m_pVertexData[index].texcoord = texcoord;
	m_pVertexData[index].normal = { normal.x * -1.0f, normal.y, normal.z };
}

void TriangleModel::Draw(ID3D12GraphicsCommandList* cmdList, int index) {
	if (m_vertexCount == 0 || !m_pVertexResource) return;

	// 三角形リストとしてトポロジを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetGraphicsRootConstantBufferView(0, m_pMaterialResource->GetGPUVirtualAddress());
	// 頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	// 全体のバッファのうち、指定されたindex番目の三角形（3頂点）を描画
	cmdList->DrawInstanced(3, 1, index * 3, 0);
}


