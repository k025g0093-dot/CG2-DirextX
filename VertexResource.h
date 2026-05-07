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
#include"Vector4.h"



// VertexResource.h
ID3D12Resource* CreateVertexResource(
	ID3D12Device* device,
	size_t sizeInBytes,
	HRESULT& hr);

D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(
	ID3D12Resource* vertexResource,
	size_t sizeInBytes,
	size_t strideInBytes);


ID3D12Resource* CreateBufferResource(
	ID3D12Device* device,
	size_t sizeInBytes
);
