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

#include <wrl.h>
#include <string>
#include <array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"DbgHelp.lib")
// TextureManager.h などのライブラリリンク部分に追加
#pragma comment(lib, "DirectXTex.lib")

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <vector>
#include <map>
#include "ConvertString.h"
#include "LogSistem.h"
#include "VertexResource.h"

class TextureManager
{

public:

    static TextureManager* GetInstance() {
        static TextureManager instance;
        return &instance;
    }

    static const int MAX_TEXTURES = 256;
    static const int IMGUI_RESERVED = 1; // ★追加：ImGuiが0番を使うので1個分予約
    int m_textureCount = 0;

    TextureManager() = default;
    ~TextureManager() = default;
    void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap,
        ID3D12GraphicsCommandList* commandList);

    int LoadTexture(const std::string& filePath);

    ID3D12DescriptorHeap* GetSRVHeap() const { return m_srvHeap; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
        uint32_t descriptorSize,
        uint32_t index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
        uint32_t descriptorSize,
        uint32_t index);
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap; }
    ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;  // SRV(テクスチャ等)用

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() const { return textureSrvHandleGPU; }
private:



    // テクスチャリソースの作成（内部処理用）
    ID3D12Resource* CreateTextureResource(const DirectX::TexMetadata& metadata);
    void CreateTextureSRV(ID3D12Resource* textureResource, const DirectX::TexMetadata& metadata,int m_textureCount); // SRVの設定情報を作る
    ID3D12Resource* UploadTexture(
        ID3D12Resource* texture,
        const DirectX::ScratchImage& mipImages);


    std::map<std::wstring, int> m_filePathToIndexMap;
    ID3D12Device* m_device = nullptr;  
    ID3D12DescriptorHeap* m_srvHeap = nullptr;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, MAX_TEXTURES> m_textures;
    UINT m_descriptorSize = 0;

    IDXGIFactory7* dxgiFactory = nullptr;        // アダプター列挙用ファクトリ
    ID3D12Device* device = nullptr;              // デバイス（心臓部）
    ID3D12CommandAllocator* commandAllocator = nullptr; // コマンドメモリ確保用
    ID3D12GraphicsCommandList* m_commandList = nullptr;   // GPUへの命令記録用
    HRESULT hr = S_OK;                           // 各種関数の成否チェック用

    // --- 6. テクスチャ・リソース作成用（追加分） ---
    D3D12_RESOURCE_DESC resourceDesc{};   // リソースの詳細設定
    D3D12_RESOURCE_DESC depthResourceDesc{};//深度バッファ用
    ID3D12Resource* resource = nullptr;                 // 汎用リソースポインタ
    ID3D12Resource* texture = nullptr;          // テクスチャ用リソースポインタ
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};    // テクスチャ用のGPUハンドルを保持する変数

    ID3D12Resource* intermediateResource{};

    uint32_t descriptorSizeSRV{};

};

