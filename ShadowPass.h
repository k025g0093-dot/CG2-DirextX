#pragma once
#include "ShadowMapBuffer.h"
#include "PSO.h"
#include "allVector.h"
#include "LightManager.h"

using Microsoft::WRL::ComPtr;

class ShadowPass
{

public:
	bool Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap,
		ComPtr<ID3D12RootSignature>& rootSig);

	void Begin(ID3D12GraphicsCommandList* cmd,
		ID3D12RootSignature* rootSig,
		ID3D12DescriptorHeap* heap);

	void End(ID3D12GraphicsCommandList* cmd);

	void Update(const LightData& light);

	void BindForMainPass(ID3D12GraphicsCommandList* cmd);

private:

	//shadow.VSに送るためのパイプ設定だったり
	ComPtr<ID3D12PipelineState> m_shadowPipelineState;
	ComPtr<ID3D12Resource> m_lightVPBuffer;

	HRESULT hr = S_OK;

	ShadowMapBuffer* shadowBuf = nullptr;

	Matrix4x4* m_mappedLightVP = nullptr;

	uint32_t Align256(uint32_t size)
	{
		return (size + 0xff) & ~0xff;
	}
};

