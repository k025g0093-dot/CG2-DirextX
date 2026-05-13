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


void TUFEngine::InitWindow() {
	// 1. ウィンドウクラスの登録
	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"MyWindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	// 2. クライアント領域のサイズからウィンドウ全体のサイズを計算
	// width と height はクラスのメンバ変数として保持しているものを使います
	RECT wrc = { 0, 0, width, height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// 3. ウィンドウ作成
	// 作成したハンドルはメンバ変数の hwnd に格納します
	hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	assert(hwnd != nullptr); // 作成に失敗していないかチェック
}


TUFEngine::TUFEngine(int32_t width, int32_t height, std::wstring name)
	: width(width), height(height) {

	// --- 1. システム基盤の初期化 ---
	// COMは一番最初に初期化しておくのが Windows プログラミングの定石
	HRESULT hrCo = CoInitializeEx(0, COINIT_MULTITHREADED);

	// フォルダ作成とログ初期化
	std::filesystem::create_directory("logs");
	InitializeLog();

	// --- 2. ウィンドウとレンダラーの準備 ---
	InitWindow();

#ifdef _DEBUG
	EnableDebugLayer(); // デバッグレイヤーはデバイス作成前に呼ぶ必要があるためここがベスト
#endif

	InitializeDXGI(hwnd); // ここで device, rootSignature などが作られる

#ifdef _DEBUG
	SetupInfoQueue();
#endif

	// --- 3. 描画ルールの構築 ---
	// InitializeDXGI で device と rootSignature が作られた後に実行する
	pipelineState = CreatePipelineStateDesc(device, rootSignature, hr);

#ifdef USE_IMGUI
	InitializeImGui(hwnd);
#endif

	CreateDepthStencilTextureResource(width, height);

	descriptorSizeSRV = device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

	 descriptorSizeRTV = device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);

	 descriptorSizeDSV = device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV
		);
}

TUFEngine::~TUFEngine() {
	pipelineState->Release();
	rootSignature->Release();
	rtvDescriptorHeap->Release();
	srvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	dxgiFactory->Release();
	device->Release();
	logStream.close();
	CoUninitialize();
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

#pragma region DirectX 12 初期化関連

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
#pragma endregion


//ファイルの読み込みトミニマップの作成、そして転送まで
#pragma region テクスチャのロード
ID3D12Resource* TUFEngine::LoadTexture(const std::string& filePath) {

	// ① テクスチャファイル読み込み
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ② ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	ID3D12Resource* texResource = CreateTextureResource(mipImages.GetMetadata());

	UploadTexture(texResource, mipImages);

	//SRVの作成
	CreateTextureSRV(texResource, mipImages.GetMetadata());

	return texResource;
}


ID3D12Resource* TUFEngine::CreateTextureResource(
	const DirectX::TexMetadata& metadata) {
	resourceDesc = {};
	resourceDesc.Width = static_cast<UINT>(metadata.width);
	resourceDesc.Height = static_cast<UINT>(metadata.height);
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);

	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	resource = {};
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,//データ転送設定
		nullptr,
		IID_PPV_ARGS(&resource)//作成するリソースポインタへ
	);
	assert(SUCCEEDED(hr));
	return resource;
}


[[nodiscard]]//属性定義：この関数の戻り値を無視しないでね、という意味
ID3D12Resource* TUFEngine::UploadTexture(
	ID3D12Resource* texture,
	const DirectX::ScratchImage& mipImages)
{
	//中間バッファの作成と転送の準備
	std::vector<D3D12_SUBRESOURCE_DATA>subresources;

	DirectX::PrepareUpload(
		device,
		mipImages.GetImages(),
		mipImages.GetImageCount(),
		mipImages.GetMetadata(),
		subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
	
	//データ転送コマンドの作成と積み込み
	UpdateSubresources(
		commandList, texture,
		intermediateResource, 0, 0, 
		UINT(subresources.size()), subresources.data()
	);

	//Tetrueへの転送が終わったら、テクスチャの使用目的をコピー先からシェーダーリソースへ変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;

}


ID3D12Resource* TUFEngine::CreateDepthStencilTextureResource(
	int32_t width, int32_t height)
{
	depthResourceDesc = {};
	depthResourceDesc.Width = width;//幅
	depthResourceDesc.Height = height;//高さ
	depthResourceDesc.MipLevels = 1;//mipmapの数
	depthResourceDesc.DepthOrArraySize = 1;//深度バッファーは配列もミップマップも不要
	depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//利用可能なフォーマット
	depthResourceDesc.SampleDesc.Count = 1;//1固定のサンプリングカウント
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2Dテクスチャ
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//深度バッファーとして使うためのフラグ

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//深度バッファーは1.0で初期化
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilResource = {};
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue, // ← 正しい
		IID_PPV_ARGS(&depthStencilResource)
	);
	assert(SUCCEEDED(hr));
	
	//DSV用のヒープ作成
	dsvDescriptorHeap = CreateDescriptorHeap(
		device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはリソースに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(
		depthStencilResource, &dsvDesc,
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


	return depthStencilResource;

}



#pragma endregion

#pragma region 描画のコマンド

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

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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

#pragma endregion

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


#ifdef _DEBUG
// --- デバッグ層：エラーがあった時にコンソールに詳細を出してくれる機能 ---
void TUFEngine::EnableDebugLayer() {
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
		debugController->Release();
	}
}

// --- メッセージフィルタ：特定の警告や情報を無視する設定 ---
void TUFEngine::SetupInfoQueue() {
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);
		infoQueue->Release();
	}
}
#endif

//SRVの作成関数
void TUFEngine::CreateTextureSRV(
	ID3D12Resource* textureResource,
	const DirectX::TexMetadata& metadata)
{

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV,1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);

	//metaDataをもとにSRVの設定を行う
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);

	//先頭ではIMGUIが使ってるのでその次を使用
	textureSrvHandleCPU.ptr += device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

	textureSrvHandleGPU.ptr += device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

	textureSrvHandleCPU2.ptr += device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

	textureSrvHandleGPU2.ptr += device->
		GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		);

	this->textureSrvHandleGPU = textureSrvHandleGPU;
	this->textureSrvHandleGPU2 = textureSrvHandleGPU2;
	device->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);
	device->CreateShaderResourceView(textureResource, &srvDesc2, textureSrvHandleCPU2);
}

#pragma region GPUとCPUのハンドルを取得

D3D12_CPU_DESCRIPTOR_HANDLE TUFEngine::GetCPUDescriptorHandle(
	ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index
) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE TUFEngine::GetGPUDescriptorHandle(
	ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index
) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}
