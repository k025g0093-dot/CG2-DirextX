#include "LightManager.h"
#include "VertexResource.h" // CreateBufferResource がここにある想定

LightManager* LightManager::s_instance = nullptr;

LightManager* LightManager::GetInstance() {
    if (!s_instance) s_instance = new LightManager();
    return s_instance;
}

void LightManager::Initialize(ID3D12Device* device) {
    m_device = device;

    m_globalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_globalLight.direction = { 0.0f, -1.0f, 0.0f };
    m_globalLight.intensity = 1.0f;

    m_globalCB = CreateLightCB(device, m_globalLight);
}

void LightManager::SetGlobalLight(const DirectionalLight& light) {
    m_globalLight = light;
    UploadGlobal();
}

void LightManager::SetPerObjectLight(int id, const DirectionalLight& light) {
    m_perObjectLights[id] = light;

    if (m_perObjectCBs.find(id) == m_perObjectCBs.end()) {
        m_perObjectCBs[id] = CreateLightCB(m_device, light);
    }
    else {
        UploadPerObject(id, light);
    }
}

void LightManager::ClearPerObjectLight(int id) {
    m_perObjectLights.erase(id);
    m_perObjectCBs.erase(id);
}

bool LightManager::HasPerObjectLight(int id) const {
    return m_perObjectLights.find(id) != m_perObjectLights.end();
}

void LightManager::Bind(ID3D12GraphicsCommandList* cmdList, int id) {
    ID3D12Resource* cb = nullptr;

    if (id == 0) {
        cb = m_globalCB.Get();
    }
    else if (id > 0) {
        auto it = m_perObjectCBs.find(id);
        if (it != m_perObjectCBs.end()) {
            cb = it->second.Get();
        }
    }

    if (!cb) {
        cb = m_globalCB.Get();
    }

    if (cb) {
        cmdList->SetGraphicsRootConstantBufferView(3, cb->GetGPUVirtualAddress());
    }
}

void LightManager::UploadGlobal() {
    if (!m_globalCB) return;
    DirectionalLight* p = nullptr;
    m_globalCB->Map(0, nullptr, reinterpret_cast<void**>(&p));
    *p = m_globalLight;
    m_globalCB->Unmap(0, nullptr);
}

void LightManager::UploadPerObject(int id, const DirectionalLight& light) {
    auto it = m_perObjectCBs.find(id);
    if (it == m_perObjectCBs.end()) return;
    DirectionalLight* p = nullptr;
    it->second->Map(0, nullptr, reinterpret_cast<void**>(&p));
    *p = light;
    it->second->Unmap(0, nullptr);
}

ComPtr<ID3D12Resource> LightManager::CreateLightCB(ID3D12Device* device, const DirectionalLight& data) {
    auto cb = CreateBufferResource(device, (sizeof(DirectionalLight) + 255) & ~255);
    DirectionalLight* p = nullptr;
    cb->Map(0, nullptr, reinterpret_cast<void**>(&p));
    *p = data;
    cb->Unmap(0, nullptr);
    return cb;
}

// LightManager.cpp に追加
std::vector<int> LightManager::GetPerObjectIds()  {
    std::vector<int> ids;
    for (auto& [id, _] : m_perObjectLights) ids.push_back(id);
    return ids;
}

DirectionalLight LightManager::GetPerObjectLight(int id)  {
    auto it = m_perObjectLights.find(id);
    if (it != m_perObjectLights.end()) return it->second;
    return m_globalLight; // フォールバック
}

