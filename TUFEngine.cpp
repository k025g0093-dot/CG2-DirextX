#include "TUFEngine.h"


// --- ウィンドウプロシージャ：Windowsからのメッセージ（閉じるボタンなど）を処理 ---
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
#ifdef USE_IMGUI

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}
#endif // USE_IMGUI

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}



TUFEngine::TUFEngine(int32_t width, int32_t height, HWND hwnd)
	: width(width), height(height) {

	std::filesystem::create_directory("logs");
	InitializeLog();
	InitializeDXGI(hwnd);

#ifdef USE_IMGUI
	InitializeImGui(hwnd);
#endif // USE_IMGUI

	pipelineState = CreatePipelineStateDesc(device, rootSignature, hr);
}

TUFEngine::~TUFEngine() {
	pipelineState->Release();
	rootSignature->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	dxgiFactory->Release();
	device->Release();
	logStream.close();
}

#ifdef USE_IMGUI

void TUFEngine::InitializeImGui(HWND hwnd) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device,
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
		);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
}
#endif // USE_IMGUI

void TUFEngine::InitializeDXGI(HWND hwnd) {
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	IDXGIAdapter4* useAdapter = nullptr;
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter))
		!= DXGI_ERROR_NOT_FOUND; i++) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(logStream, ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			Log(logStream, std::format("Feature Level {} is supported.\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device != nullptr);
	Log(logStream, "Complete DirectX 12 Device Creation.\n");

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));

	swapChainDesc;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd,
		&swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	
	srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);


		hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	rtvDesc;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle =
		rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	rtvHandles[1].ptr = rtvHandles[0].ptr +
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	useAdapter->Release();
}

void TUFEngine::PreDraw() {
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources[backBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	D3D12_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = width;
	scissorRect.top = 0;
	scissorRect.bottom = height;
	commandList->RSSetScissorRects(1, &scissorRect);
}

void TUFEngine::PostDraw() {
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources[backBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
#ifdef USE_IMGUI
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // USE_IMGUI

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);

	hr = commandList->Close();
	assert(SUCCEEDED(hr));
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);
	swapChain->Present(1, 0);

	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	fenceValue++;
	commandQueue->Signal(fence, fenceValue);
	if (fence->GetCompletedValue() < fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	CloseHandle(fenceEvent);
	fence->Release();

	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));
}


// 修正後：関数名の前に TUFEngine:: を追加
ID3D12DescriptorHeap* TUFEngine::CreateDescriptorHeap(
    ID3D12Device* device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType,
    uint32_t numDescriptors,
    bool shaderVisible)
{
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = heapType;
    descriptorHeapDesc.NumDescriptors = numDescriptors;
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    assert(SUCCEEDED(hr));
    
    return descriptorHeap;
}
