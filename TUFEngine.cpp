#include "TUFEngine.h"


void InitializeEngine(ID3D12Device* device, HRESULT& hr) {
    
    std::filesystem::create_directory("logs");
    InitializeLog();

    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = CreatePipelineStateDesc(device, rootSignature, hr);
}



void FinalizeEngine() {
    logStream.close();
}