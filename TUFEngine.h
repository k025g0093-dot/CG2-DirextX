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

#ifdef USE_IMGUI

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // USE_IMGUI



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


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

	ID3D12DescriptorHeap* CreateDescriptorHeap(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType,
        uint32_t numDescriptors,
        bool shaderVisible
        );


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
    ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;
    ID3D12Resource* swapChainResources[2] = { nullptr };
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2]{};
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    HRESULT hr = S_OK;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

    void InitializeDXGI(HWND hwnd);
	void InitializeImGui(HWND hwnd);
};