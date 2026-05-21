#pragma once
#include "AllIncludeHeder.h"
#include "DynamicMeshModel.h"
struct VertexData {
    Vector4 position;
    Vector2 texcoord; // テクスチャのどこを使うかの指定
    Vector3 normal;
};

struct Material {
    Vector4 color;
    int32_t enableLifhting;
    float padding[3];
    Matrix4x4 uvTransform;
};

struct TransformationMatrix
{
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct DirectionalLLight
{
    Vector4 color;
    Vector3 direction;
    float intensity;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


// TUFEngine.h などの適切な場所に配置
struct DrawRequest {
    Model* model = nullptr;
    std::vector<Vector3> vertices;
    Vector4 color = { 1, 1, 1, 1 };
    std::vector<Vector2> uvs;
    Vector3 rot = { 0, 0, 0 };
    Vector3 scale = { 1, 1, 1 };
    Vector3 pos = { 0, 0, 0 };
    int textureIndex = 0; // ★超重要：ゴミデータが入るのを防ぐ
    bool isMesh = false;  // ★超重要：メッシュ判定が狂うのを防ぐ
};


class TUFEngine {
public:
    TUFEngine(int32_t width, int32_t height, std::wstring name);
    ~TUFEngine();
    static TUFEngine* GetInstance() { return s_instance; }
    void OnUpdate();
    MeshModel* LoadModel(const std::string& directoryPath, const std::string& filename);

    void DrawTriangle(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color);
    void DrawSphere(const Vector3& pos, const Vector3& rot, const Vector3& scale, int textureIndex);
    void DrawMesh(MeshModel* mesh, Vector3 pos, Vector3 rot, Vector3 scale);
    void DrawMeshTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector4 color, std::vector<Vector2> uvs, Vector3 rot, Vector3 scale, int index);


    void DrawDynamicMesh(DynamicMesh& mesh, Vector4 color);
    void DrawDynamicMeshWithNormal(DynamicMesh& mesh,
        std::vector<Vector4>& colors);


    void PreDraw();
    void PostDraw();

    ID3D12Device* GetDevice() { return device; }
    ID3D12GraphicsCommandList* GetCommandList() { return commandList; }
    ID3D12RootSignature* GetRootSignature() { return rootSignature; }
    ID3D12PipelineState* GetPipelineState() { return pipelineState; }
    HWND GetHwnd() const { return hwnd; }
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap; }
    ID3D12DescriptorHeap* GetDsvDescriptorHeap() const { return dsvDescriptorHeap; }

    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU() const { return textureSrvHandleGPU; }


    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
        ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index
    );

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
        ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index
    );

    void EnableDebugLayer();
    void SetupInfoQueue();

    ID3D12DescriptorHeap* CreateDescriptorHeap(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType,
        uint32_t numDescriptors,
        bool shaderVisible
    );

    const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }

    // ⭕ 2. カメラ行列のセッター（main.cpp などから最新のカメラ行列をセットするため）
    void SetViewProjectionMatrix(const Matrix4x4& vp) { viewProjectionMatrix = vp; }
    void SetDirectionalLightResource(ID3D12Resource* lightResource) { m_directionalLightResource = lightResource; }

    Camera m_camera;

private:

   
    static TUFEngine* s_instance;
    int m_cbvIndex = 0; // 今何個目の三角形を描いているかのカウント
    static const int MAX_DRAW_COUNT = 1000000; // 1フレームに描ける最大数
    UINT8* m_pCbvDataBegin = nullptr;        // 1バイト単位で計算できるように UINT8* にする
    std::unique_ptr<ImGuiUIManager> m_imguiManager;



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
    D3D12_RESOURCE_DESC resourceDesc{};   // リソースの詳細設定
    D3D12_RESOURCE_DESC depthResourceDesc{};//深度バッファ用
    ID3D12Resource* resource = nullptr;                 // 汎用リソースポインタ
    ID3D12Resource* texture = nullptr;          // テクスチャ用リソースポインタ
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};    // テクスチャ用のGPUハンドルを保持する変数

    ID3D12Resource* intermediateResource{};
    ID3D12Resource* depthStencilResource = nullptr;//深度バッファ専用のリソース
    ID3D12DescriptorHeap* dsvDescriptorHeap = {}; // 深度ステンシルビュー用のデスクリプタヒープ
    uint32_t descriptorSizeSRV{};

    uint32_t descriptorSizeRTV{};

    uint32_t descriptorSizeDSV{};

    VertexData* vertexData{};

    Matrix4x4 viewProjectionMatrix;

    // --- 内部初期化用メソッド ---
    void InitWindow();                                  // 窓を作る
    void InitializeDXGI(HWND hwnd);                     // DX12の基本初期化
    void InitializeImGui(HWND hwnd);                    // ImGuiの初期化

    // テクスチャリソースの作成（内部処理用）
    ID3D12Resource* CreateDepthStencilTextureResource(int32_t width, int32_t height);//深度バッファーのリソース作成
    void RenderAllRequests();


    Sphere sphere_;
    TriangleModel triangle;
    MeshModel* meshModel_;
    std::unique_ptr<DynamicMeshModel> m_dynamicMeshModel;


    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource>      m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                   m_vertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource>      m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeapImgui;
    D3D12_VIEWPORT m_viewport; // ビューポートの定義  
    D3D12_RECT m_scissorRect; // シザー矩形の定義
    UINT m_constantBufferDescriptorSize = 0;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    ID3D12Resource* m_pConstantBuffer = nullptr; // ⭕ 一括管理用の巨大な定数バッファ
    ID3D12Resource* m_directionalLightResource = nullptr;
    std::vector<DrawRequest> m_drawRequests;

    std::unique_ptr<Sphere>m_temporarySpheres;
    std::unique_ptr<TriangleModel>m_temporaryTriangle;

    std::map<std::string, std::unique_ptr<MeshModel>> m_meshes;


    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fenceValue = 0;
    HANDLE m_fenceEvent = nullptr;

};
