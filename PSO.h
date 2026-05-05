#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "LogSistem.h"
#include "DXC.h"
#include "ConvertString.h"
#include <format>
#include <string>  

ID3D12PipelineState* CreatePipelineStateDesc(
    ID3D12Device* device,
    ID3D12RootSignature*& rootSignature, 
    HRESULT& hr);



ID3D12RootSignature* CreateRootSignature(
    ID3D12Device* device,
    HRESULT& hr);

D3D12_INPUT_LAYOUT_DESC CreateLayout();

D3D12_BLEND_DESC CreateBlendState();

D3D12_RASTERIZER_DESC CreateRasterizerState();

