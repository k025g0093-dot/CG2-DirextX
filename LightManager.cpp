#include "LightManager.h"
#include "VertexResource.h"
#include "LogSistem.h"

LightManager* LightManager::s_instance = nullptr;

LightManager* LightManager::GetInstance() {
    if (!s_instance) s_instance = new LightManager();
    return s_instance;
}

void LightManager::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {
    m_device = device;

    m_globalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_globalLight.direction = { 0.0f, -1.0f, 0.0f };
    m_globalLight.intensity = 1.0f;

    UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    UINT bufferSize = sizeof(LightData) * MAX_LIGHTS;
    m_lightBuffer = CreateBufferResource(device, bufferSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = MAX_LIGHTS;
    srvDesc.Buffer.StructureByteStride = sizeof(LightData);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = srvHeap->GetCPUDescriptorHandleForHeapStart().ptr + descriptorSize * LIGHT_SRV_SLOT;
    device->CreateShaderResourceView(m_lightBuffer.Get(), &srvDesc, cpuHandle);

    m_lightSrvGpuHandle.ptr = srvHeap->GetGPUDescriptorHandleForHeapStart().ptr + descriptorSize * LIGHT_SRV_SLOT;

    Upload();
}

void LightManager::SetGlobalLight(const DirectionalLight& light) {
    m_globalLight = light;
    Upload();
}

void LightManager::Bind(ID3D12GraphicsCommandList* cmdList, int id) {
    cmdList->SetGraphicsRootDescriptorTable(3, m_lightSrvGpuHandle);
}

void LightManager::Upload() {
    if (!m_lightBuffer) return;
    m_lights[0].dirOrPos = { m_globalLight.direction.x, m_globalLight.direction.y, m_globalLight.direction.z };
    m_lights[0].color = { m_globalLight.color.x, m_globalLight.color.y, m_globalLight.color.z };
    m_lights[0].intensity = m_globalLight.intensity;

    void* p = nullptr;
    HRESULT hr = m_lightBuffer->Map(0, nullptr, &p);
    if (SUCCEEDED(hr) && p) {
        memcpy(p, m_lights, sizeof(LightData) * MAX_LIGHTS);
        m_lightBuffer->Unmap(0, nullptr);
    }
}
