#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <map>
#include <vector>
#include "allVector.h"

using Microsoft::WRL::ComPtr;

struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

struct PointLight {
    Vector4 color;//ライトの色
    Vector3 direction;//ライトの位置
    float intensity;//光度
};

class LightManager {
public:
    static LightManager* GetInstance();

    void Initialize(ID3D12Device* device);

    // グローバルライト
    void SetGlobalLight(const DirectionalLight& light);
    const DirectionalLight& GetGlobalLight() const { return m_globalLight; }

    // 個別ライト
    void SetPerObjectLight(int id, const DirectionalLight& light);
    void ClearPerObjectLight(int id);
    bool HasPerObjectLight(int id) const;

    // 描画コマンドにセットする直前に呼ぶ
    // 個別があればそちら、なければグローバルをb1にバインドする
    void Bind(ID3D12GraphicsCommandList* cmdList, int id);

    // ImGui描画（LightManagerが直接持つ）
    void DrawImGui();


    std::vector<int> GetPerObjectIds() ;
    DirectionalLight GetPerObjectLight(int id) ;
private:
    LightManager() = default;

    void UploadGlobal();
    void UploadPerObject(int id, const DirectionalLight& light);

    static LightManager* s_instance;

    ID3D12Device* m_device = nullptr;

    DirectionalLight             m_globalLight{};
    ComPtr<ID3D12Resource>       m_globalCB;

    std::map<int, DirectionalLight>       m_perObjectLights;
    std::map<int, ComPtr<ID3D12Resource>> m_perObjectCBs;

    static ComPtr<ID3D12Resource> CreateLightCB(ID3D12Device* device, const DirectionalLight& data);
};