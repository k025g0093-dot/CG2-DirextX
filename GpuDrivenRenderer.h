#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "GpuDrivenTypes.h"

class GpuDrivenRenderer
{
public:
	void Initialize(
		ID3D12Device* device,
		ID3D12DescriptorHeap* srvDescriptorHeap,
		int maxDrawCount);

	bool IsInitialized() const { return m_device != nullptr; }
	int GetMaxDrawCount() const { return m_maxDrawCount; }

private:
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	ID3D12DescriptorHeap* m_srvDescriptorHeap = nullptr;
	int m_maxDrawCount = 0;
};
