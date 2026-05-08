#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <cassert>
#include <filesystem>
#include <dbghelp.h>
#include <strsafe.h>
#include <string>
#include <format>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"DbgHelp.lib")

#include "ConvertString.h"
#include "LogSistem.h"
#include "DXC.h"
#include "PSO.h"
#include "VertexResource.h"
#include "Vector.h"

class TUFEngine {
public:
    TUFEngine(int32_t width, int32_t height, HWND hwnd);
    ~TUFEngine();

    void PreDraw();
    void PostDraw();

    ID3D12Device* GetDevice() { return device; }
    ID3D12GraphicsCommandList* GetCommandList() { return commandList; }
    ID3D12RootSignature* GetRootSignature() { return rootSignature; }
    ID3D12PipelineState* GetPipelineState() { return pipelineState; }

    void EnableDebugLayer();
    void SetupInfoQueue();

private:
    int32_t width = 0;
    int32_t height = 0;

    IDXGIFactory7* dxgiFactory = nullptr;
    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* commandQueue = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
    IDXGISwapChain4* swapChain = nullptr;
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
    ID3D12Resource* swapChainResources[2] = { nullptr };
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2]{};
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    HRESULT hr = S_OK;

    void InitializeDXGI(HWND hwnd);
};