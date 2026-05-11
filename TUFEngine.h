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
// TUFEngine.h などのライブラリリンク部分に追加
#pragma comment(lib, "DirectXTex.lib")

#include "ConvertString.h"
#include "LogSistem.h"
#include "DXC.h"
#include "PSO.h"
#include "VertexResource.h"
#include "externals/DirectXTex/DirectXTex.h"

#include "allVector.h"

#ifdef USE_IMGUI

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // USE_IMGUI



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


class TUFEngine {
public:
    TUFEngine(int32_t width, int32_t height, std::wstring name);
    ~TUFEngine();

    void PreDraw();
    void PostDraw();

    ID3D12Device* GetDevice() { return device; }
    ID3D12GraphicsCommandList* GetCommandList() { return commandList; }
    ID3D12RootSignature* GetRootSignature() { return rootSignature; }
    ID3D12PipelineState* GetPipelineState() { return pipelineState; }
    HWND GetHwnd() const { return hwnd; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() const { return textureSrvHandleGPU; }
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap; }

    void EnableDebugLayer();
    void SetupInfoQueue();

    ID3D12DescriptorHeap* CreateDescriptorHeap(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType,
        uint32_t numDescriptors,
        bool shaderVisible
    );

    ID3D12Resource* LoadTexture(const std::string& filePath);

    void UploadTexture(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

private:
    // --- 1. ウィンドウ・システム関連 ---
    HWND hwnd = nullptr;                 // ウィンドウハンドル
    int32_t width = 0;                  // 画面の横幅
    int32_t height = 0;                 // 画面の縦幅

    // --- 2. DirectX 12 基本オブジェクト ---
    IDXGIFactory7* dxgiFactory = nullptr;        // アダプター列挙用ファクトリ
    ID3D12Device* device = nullptr;              // デバイス（心臓部）
    ID3D12CommandQueue* commandQueue = nullptr;  // コマンド実行用キュー
    ID3D12CommandAllocator* commandAllocator = nullptr; // コマンドメモリ確保用
    ID3D12GraphicsCommandList* commandList = nullptr;   // GPUへの命令記録用
    HRESULT hr = S_OK;                           // 各種関数の成否チェック用

    // --- 3. スワップチェーン & 画面出力関連 ---
    IDXGISwapChain4* swapChain = nullptr;               // 画面入れ替え制御
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};              // スワップチェーンの設定情報
    ID3D12Resource* swapChainResources[2] = { nullptr }; // バックバッファ(画面の実体)

    // --- 4. デスクリプタヒープ (住所録) ---
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;  // RTV(レンダターゲット)用
    ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;  // SRV(テクスチャ等)用
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2]{};       // RTVのハンドル（CPU側）
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};           // RTVの設定情報

    // --- 5. パイプライン・描画設定 ---
    ID3D12RootSignature* rootSignature = nullptr;       // 定数バッファ等の渡し方の定義
    ID3D12PipelineState* pipelineState = nullptr;       // シェーダーや各種描画ルール

    // --- 6. テクスチャ・リソース作成用（追加分） ---
    D3D12_RESOURCE_DESC resourceDesc{};                 // リソースの詳細設定
    ID3D12Resource* resource = nullptr;                 // 汎用リソースポインタ
    ID3D12Resource* texture = nullptr;          // テクスチャ用リソースポインタ
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};    // テクスチャ用のGPUハンドルを保持する変数


    // --- 内部初期化用メソッド ---
    void InitWindow();                                  // 窓を作る
    void InitializeDXGI(HWND hwnd);                     // DX12の基本初期化
    void InitializeImGui(HWND hwnd);                    // ImGuiの初期化

    // テクスチャリソースの作成（内部処理用）
    ID3D12Resource* CreateTextureResource(const DirectX::TexMetadata& metadata);

    void CreateTextureSRV(ID3D12Resource* textureResource, const DirectX::TexMetadata& metadata); // SRVの設定情報を作る
};