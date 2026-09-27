#include "ShadowPass.h"

bool ShadowPass::Initialize(
	ID3D12Device* device,
	ID3D12DescriptorHeap* srvHeap,
	ComPtr<ID3D12RootSignature>& rootSig)
{
	ShadowMapBuffer::GetInstance()->Initialize(device, srvHeap);
	shadowBuf = ShadowMapBuffer::GetInstance();
	m_shadowPipelineState=
		CreateShadowPipelineState(device, rootSig, hr);

	m_lightVPBuffer = CreateBufferResource(device, Align256(sizeof(Matrix4x4)));
	m_lightVPBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedLightVP));

	return true;
}

void ShadowPass::Begin(ID3D12GraphicsCommandList* cmd,
	ID3D12RootSignature* rootSig,
	ID3D12DescriptorHeap* heap) 

{
	shadowBuf->TransitionToDsv(cmd);

	cmd->SetGraphicsRootSignature(rootSig);
	cmd->SetDescriptorHeaps(1, &heap);
	cmd->SetPipelineState(m_shadowPipelineState.Get());


	D3D12_VIEWPORT shadowVP{};
	shadowVP.Width = (float)ShadowMapBuffer::SHADOW_MAP_SIZE;
	shadowVP.Height = (float)ShadowMapBuffer::SHADOW_MAP_SIZE;
	shadowVP.MaxDepth = 1.0f;
	cmd->RSSetViewports(1, &shadowVP);



	D3D12_RECT shadowRect{};
	shadowRect.right = ShadowMapBuffer::SHADOW_MAP_SIZE;
	shadowRect.bottom = ShadowMapBuffer::SHADOW_MAP_SIZE;
	cmd->RSSetScissorRects(1, &shadowRect);

	auto shadowDsv = shadowBuf->GetDsvCpuHandle();
	cmd->OMSetRenderTargets(0, nullptr, false, &shadowDsv);
	cmd->ClearDepthStencilView(shadowDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmd->SetGraphicsRootConstantBufferView(9, m_lightVPBuffer->GetGPUVirtualAddress());

}

void ShadowPass::End(ID3D12GraphicsCommandList* cmd) {

	shadowBuf->TransitionToSrv(cmd);
}

void ShadowPass::Update(const LightData& light)
{
	Vector3 lightDir = light.dirOrPos.Normalized();
	Vector3 lightPos = lightDir * -50.0f;

	Vector3 up = { 0.0f, 1.0f, 0.0f };
	if (std::abs(lightDir.Dot(up)) > 0.99f) up = { 0.0f, 0.0f, 1.0f };

	Matrix4x4 view = MakeLookAtMatrix(lightPos, { 0,0,0 }, up);
	Matrix4x4 proj = MakeOrthographicMatrix(-50.0f, 50.0f, 50.0f, -50.0f, 0.1f, 100.0f);
	*m_mappedLightVP = Multiply(view, proj);
}

void ShadowPass::BindForMainPass(ID3D12GraphicsCommandList* cmd) {
	cmd->SetGraphicsRootDescriptorTable(8, shadowBuf->GetSrvGpuHandle());
	cmd->SetGraphicsRootConstantBufferView(9, m_lightVPBuffer->GetGPUVirtualAddress());
}
