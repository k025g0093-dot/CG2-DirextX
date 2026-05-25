#pragma once
#include "AllIncludeHeder.h"
#include "DynamicMeshModel.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

struct Material {
    Vector4 color;
    int32_t enableLifhting;
    float padding[3];
    Matrix4x4 uvTransform;
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct DirectionalLLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct DrawRequest {
    Model* model = nullptr;
    std::vector<Vector3> vertices;
    Vector4            color = { 1, 1, 1, 1 };
    std::vector<Vector2> uvs;
    Vector3            rot = { 0, 0, 0 };
    Vector3            scale = { 1, 1, 1 };
    Vector3            pos = { 0, 0, 0 };
    Vector2            posV2 = { 0, 0 };
    float              width = 0;
    float              height = 0;
    int                textureIndex = 0;
    bool               isMesh = false;
    bool               isSprite = false;
};

class TUFEngine {
public:
    TUFEngine(int32_t width, int32_t height, std::wstring name);
    ~TUFEngine();
    static TUFEngine* GetInstance() { return s_instance; }
    void OnUpdate();

    int        LoadTexture(const std::string& filePath);
    MeshModel* LoadModel(const std::string& directoryPath, const std::string& filename);

    void DrawTriangle(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color);
    void DrawSphere(const Vector3& pos, const Vector3& rot, const Vector3& scale, int textureIndex);
    void DrawMesh(MeshModel* mesh, Vector3 pos, Vector3 rot, Vector3 scale);
    void DrawSprite(const Vector2& pos, const float width, const float height,
        const Vector3& rot, const Vector3& scale, const Vector4 color, int textureIndex);
    void DrawDynamicMesh(DynamicMesh& mesh, Vector4 color);
    void DrawDynamicMeshWithNormal(DynamicMesh& mesh, std::vector<Vector4>& colors, int index);

    void PreDraw();
    void PostDraw();

    // ゲッターは生ポインタで返す（呼び出し側に所有権を渡さない）
    ID3D12Device* GetDevice() { return device.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }
    ID3D12RootSignature* GetRootSignature() { return rootSignature.Get(); }
    ID3D12PipelineState* GetPipelineState() { return pipelineState.Get(); }
    HWND                       GetHwnd()       const { return hwnd; }
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap.Get(); }
    ID3D12DescriptorHeap* GetDsvDescriptorHeap() const { return dsvDescriptorHeap.Get(); }

    void EnableDebugLayer();
    void SetupInfoQueue();

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType,
        uint32_t numDescriptors,
        bool shaderVisible);

    const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }
    void SetViewProjectionMatrix(const Matrix4x4& vp) { viewProjectionMatrix = vp; }
    void SetDirectionalLightResource(ID3D12Resource* lightResource) { m_directionalLightResource = lightResource; }
    const Vector2& GetSpriteUVScale() const { return m_spriteUVScale; }
    const Vector2& GetSpriteUVTranslate() const { return m_spriteUVTranslate; }
    float GetSpriteUVRotate() const { return m_spriteUVRotate; }
    void SetSpriteUVScale(const Vector2& scale) { m_spriteUVScale = scale; }
    void SetSpriteUVTranslate(const Vector2& translate) { m_spriteUVTranslate = translate; }
    void SetSpriteUVRotate(float rotate) { m_spriteUVRotate = rotate; }
    void ResetSpriteUVTransform();
    Matrix4x4 GetSpriteUVTransformMatrix() const;

    Camera m_camera;

private:
    static TUFEngine* s_instance;

    int          m_cbvIndex = 0;
    static const int MAX_DRAW_COUNT = 10000;
    UINT8* m_pCbvDataBegin = nullptr;

    std::unique_ptr<ImGuiUIManager> m_imguiManager;

    // ウィンドウ
    HWND    hwnd = nullptr;
    int32_t width = 0;
    int32_t height = 0;

    // DirectX 12 基本オブジェクト（Engine が所有 → ComPtr）
    ComPtr<IDXGIFactory7>             dxgiFactory;
    ComPtr<ID3D12Device>              device;
    ComPtr<ID3D12CommandQueue>        commandQueue;
    ComPtr<ID3D12CommandAllocator>    commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    HRESULT hr = S_OK;

    // スワップチェーン
    ComPtr<IDXGISwapChain4>  swapChain;
    DXGI_SWAP_CHAIN_DESC1    swapChainDesc{};
    ComPtr<ID3D12Resource>   swapChainResources[2];

    // デスクリプタヒープ
    ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
    ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE  rtvHandles[2]{};
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

    // パイプライン
    // ※ 元コードに生ポインタ版と ComPtr 版が二重に存在していたため、ComPtr 版に統一
    ComPtr<ID3D12RootSignature>  rootSignature;
    ComPtr<ID3D12PipelineState>  pipelineState;

    // リソース類
    ComPtr<ID3D12Resource> depthStencilResource;

    ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

    Matrix4x4   viewProjectionMatrix;
    Vector2     m_spriteUVScale = { 1.0f, 1.0f };
    Vector2     m_spriteUVTranslate = { 0.0f, 0.0f };
    float       m_spriteUVRotate = 0.0f;

    // 内部初期化
    void InitWindow(std::wstring name);
    void InitializeDXGI(HWND hwnd);
    void InitializeImGui(HWND hwnd);
    ID3D12Resource* CreateDepthStencilTextureResource(int32_t width, int32_t height);
    void RenderAllRequests();

    // 描画オブジェクト
    std::unique_ptr<DynamicMeshModel> m_dynamicMeshModel;
    std::unique_ptr<Sprite>           sprite;

    // 頂点・定数バッファ（ComPtr に統一、重複していた生ポインタ版を削除）

    // ライトリソースは外部から SetDirectionalLightResource() で差し込む設計のため生ポインタを維持
    ComPtr<ID3D12Resource> m_pConstantBuffer;
    ID3D12Resource* m_directionalLightResource = nullptr;

    std::vector<DrawRequest> m_drawRequests;

    std::unique_ptr<Sphere>       m_temporarySpheres;
    std::unique_ptr<TriangleModel> m_temporaryTriangle;
    std::map<std::string, std::unique_ptr<MeshModel>> m_meshes;

    ComPtr<ID3D12Fence> m_fence;
    uint64_t            m_fenceValue = 0;
    HANDLE              m_fenceEvent = nullptr;
};
