#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "allVector.h"

using Microsoft::WRL::ComPtr;

struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

struct LightData {
    Vector3 dirOrPos;
    Vector3 color;
    float intensity;
};

class LightManager {
public:
    static LightManager* GetInstance();

    static const int MAX_LIGHTS = 8;
    static const int LIGHT_SRV_SLOT = 108;

    void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);

    void SetGlobalLight(const DirectionalLight& light);
    const DirectionalLight& GetGlobalLight() const { return m_globalLight; }

    void Bind(ID3D12GraphicsCommandList* cmdList, int id);

private:
    LightManager() = default;
    void Upload();

    static LightManager* s_instance;

    ID3D12Device* m_device = nullptr;
    DirectionalLight m_globalLight{};
    ComPtr<ID3D12Resource> m_lightBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE m_lightSrvGpuHandle{};

    LightData m_lights[MAX_LIGHTS] = {};
};
