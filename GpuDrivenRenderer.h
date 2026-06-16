#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "GpuDrivenTypes.h"

using Microsoft::WRL::ComPtr;

// 前方宣言
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(
	ID3D12Device* device,
	size_t size,
	D3D12_HEAP_TYPE heapType ,
	D3D12_RESOURCE_FLAGS flags );

class GpuDrivenRenderer
{
public:
	void Initialize(
		ID3D12Device* device,
		ID3D12DescriptorHeap* srvDescriptorHeap,
		int maxDrawCount);

	bool IsInitialized() const { return m_device != nullptr; }
	int GetMaxDrawCount() const { return m_maxDrawCount; }
	void SetMaxDrawCount(int count) { m_maxDrawCount = count; }

	// ============================================================
	// 【バッファアクセス用 Getter】
	// ============================================================
	RawTransform* GetMappedTransformData() const { return m_mappedTransformData; }
	ID3D12Resource* GetInstanceBuffer() const { return m_instanceBuffer.Get(); }
	ID3D12Resource* GetTransformBuffer() const { return m_transformBuffer.Get(); }

	// ============================================================
	// 【ディスクリプタハンドル取得】
	// ============================================================
	D3D12_GPU_DESCRIPTOR_HANDLE GetTransformSrvGpuHandle() const { return m_transformSrvGpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetInstanceUavGpuHandle() const { return m_instanceUavGpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetInstanceSrvGpuHandle() const { return m_instanceSrvGpuHandle; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetTransformSrvCpuHandle() const { return m_transformSrvCpuHandle; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetInstanceUavCpuHandle() const { return m_instanceUavCpuHandle; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetInstanceSrvCpuHandle() const { return m_instanceSrvCpuHandle; }

	// ============================================================
	// 【リソース状態管理】
	// ============================================================
	D3D12_RESOURCE_STATES GetInstanceBufferState() const { return m_instanceBufferState; }
	void SetInstanceBufferState(D3D12_RESOURCE_STATES state) { m_instanceBufferState = state; }

	// ============================================================
	// 【バッファ拡張】
	// ============================================================
	void GrowBuffers(
		ID3D12CommandQueue* commandQueue,
		ID3D12Fence* fence,
		HANDLE fenceEvent,
		uint64_t& fenceValue,
		int requiredCount);

	// ============================================================
	// 【バリア処理メソッド】
	// ============================================================
	void TransitionToUAV(ID3D12GraphicsCommandList* cmdList);
	void TransitionToSRV(ID3D12GraphicsCommandList* cmdList);

private:
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	ID3D12DescriptorHeap* m_srvDescriptorHeap = nullptr;
	int m_maxDrawCount = 0;

	// ============================================================
	// 【バッファリソース】
	// ============================================================
	ComPtr<ID3D12Resource> m_transformBuffer;      // 入力用（UPLOAD ヒープ）
	ComPtr<ID3D12Resource> m_instanceBuffer;       // 出力用（DEFAULT ヒープ + UAV）

	// ============================================================
	// 【マップされたポインタ】
	// ============================================================
	RawTransform* m_mappedTransformData = nullptr;  // CPU書き込み用

	// ============================================================
	// 【リソース状態管理】
	// ============================================================
	D3D12_RESOURCE_STATES m_instanceBufferState = D3D12_RESOURCE_STATE_COMMON;

	// ============================================================
	// 【ディスクリプタハンドル】6つ
	// ============================================================
	D3D12_CPU_DESCRIPTOR_HANDLE m_transformSrvCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_transformSrvGpuHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE m_instanceUavCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_instanceUavGpuHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE m_instanceSrvCpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_instanceSrvGpuHandle;

	// ============================================================
	// 【初期化処理用ヘルパー】
	// ============================================================
	void CreateBuffersAndViews(int newMaxDrawCount);
	void CreateDescriptorViews();
};