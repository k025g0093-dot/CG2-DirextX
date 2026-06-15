#include "GpuDrivenRenderer.h"

void GpuDrivenRenderer::Initialize(
	ID3D12Device* device,
	ID3D12DescriptorHeap* srvDescriptorHeap,
	int maxDrawCount)
{
	m_device = device;
	m_srvDescriptorHeap = srvDescriptorHeap;
	m_maxDrawCount = maxDrawCount;
}
