// TUFEngine.h
#pragma once
#include "ConvertString.h"
#include "LogSistem.h"
#include "DXC.h"
#include "PSO.h"
#include "VertexResource.h"
#include <filesystem>

void InitializeEngine(ID3D12Device* device, HRESULT& hr);
void FinalizeEngine();