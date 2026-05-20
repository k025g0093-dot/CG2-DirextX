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

#pragma region	基本的なエンジン内部の処理

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

	TextureManager::GetInstance()->Initialize(device, srvDescriptorHeap, commandList);


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

	auto sphere = std::make_unique<Sphere>();
	sphere->InitSphere(this);
	m_temporarySpheres = std::move(sphere);

	auto tri = std::make_unique<TriangleModel>();
	tri->Initialize(this);
	m_temporaryTriangle = std::move(tri);

}

MeshModel* TUFEngine::LoadModel(const std::string& directoryPath, const std::string& filename) {
	if (m_meshes.count(filename) > 0) {
		return m_meshes[filename].get();
	}

	// MeshModelを作成
	auto mesh = std::make_unique<MeshModel>();
	mesh->InitMeshModel(this);

	if (!mesh->LoadFromOBJ(directoryPath, filename)) {
		OutputDebugStringA(("Error: Failed to load OBJ: " + directoryPath + "/" + filename + "\n").c_str());
		return nullptr;
	}

	std::string baseName = filename;
	size_t lastDot = filename.find_last_of(".");
	if (lastDot != std::string::npos) {
		baseName = filename.substr(0, lastDot);
	}

	std::string folderAndBase = directoryPath + "/" + baseName;
	std::string jpgPath = folderAndBase + ".jpg";
	std::string pngPath = folderAndBase + ".png";

	std::string texPath = "";
	if (GetFileAttributesA(jpgPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
		texPath = jpgPath;
	}
	else if (GetFileAttributesA(pngPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
		texPath = pngPath;
	}

	if (!texPath.empty()) {
		int texIndex = TextureManager::GetInstance()->LoadTexture(texPath);
		mesh->SetTextureIndex(texIndex);
	}

	MeshModel* ptr = mesh.get();
	m_meshes[filename] = std::move(mesh);

	return ptr;
}


void TUFEngine::OnUpdate() {
	Input::Update();
	//sphere_.Update();
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

#pragma endregion

#ifdef USE_IMGUI

void TUFEngine::InitializeImGui(HWND hwnd) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

	// TUFEngine.cpp の初期化処理（例えば InitializeDXGI の最後など）に以下を追加
	UINT cbSize = (sizeof(TransformationMatrix) + 255) & ~255;
	// 256個分のオブジェクトの行列が入る巨大なバッファを作る
	m_pConstantBuffer = CreateBufferResource(device, cbSize * MAX_DRAW_COUNT);
	// 最初に1回だけMapして、書き込み先ポインタ（m_pCbvDataBegin）を保存しておく
	m_pConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pCbvDataBegin));

	m_fenceValue = 0;
	hr = device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	assert(SUCCEEDED(hr));

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(m_fenceEvent != nullptr);

	useAdapter->Release();
}
#pragma endregion

ID3D12Resource* TUFEngine::CreateDepthStencilTextureResource(
	int32_t width,
	int32_t height)
{
	depthResourceDesc = {};
	depthResourceDesc.Width = width;
	depthResourceDesc.Height = height;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilResource = {};
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthStencilResource)
	);
	assert(SUCCEEDED(hr));

	dsvDescriptorHeap = CreateDescriptorHeap(
		device,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		false
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device->CreateDepthStencilView(
		depthStencilResource,
		&dsvDesc,
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return depthStencilResource;
}



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

	RenderAllRequests();

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

	// ⭕ 毎フレームの新規作成/破棄を全削除！既存のフェンスを使い回してGPUを待つ
	m_fenceValue++;
	commandQueue->Signal(m_fence.Get(), m_fenceValue);

	if (m_fence->GetCompletedValue() < m_fenceValue) {
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

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

#pragma region	レンダーリクエスト

void TUFEngine::RenderAllRequests() {
	// ⭕ GPUに「このエンジンのシェーダーと設計図を使うよ」と教える
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	// テクスチャ用などのデスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = { srvDescriptorHeap };
	commandList->SetDescriptorHeaps(1, heaps);

	// 2. カメラ行列
	Matrix4x4 viewProjMatrix = viewProjectionMatrix;

	m_cbvIndex = 0;
	UINT cbSize = (sizeof(TransformationMatrix) + 255) & ~255;
	for (auto& request : m_drawRequests) {
		if (m_cbvIndex >= MAX_DRAW_COUNT) {
			break;
		}

		if (!request.model) continue;

		// --- 3. WVP行列を作成 ---
		Matrix4x4 world = MakeAffineMatrix(request.scale, request.rot, request.pos);
		Matrix4x4 wvp = Multiply(world, viewProjMatrix);

		TransformationMatrix cbData{};
		cbData.WVP = wvp;
		cbData.World = world;

		if (m_pCbvDataBegin && m_pConstantBuffer) {
			UINT8* pDest = m_pCbvDataBegin + (cbSize * m_cbvIndex);
			memcpy(pDest, &cbData, sizeof(TransformationMatrix));

			D3D12_GPU_VIRTUAL_ADDRESS cbAddr =
				m_pConstantBuffer->GetGPUVirtualAddress() + (cbSize * m_cbvIndex);
			commandList->SetGraphicsRootConstantBufferView(1, cbAddr);
		}

		// --- 6. 描画実行 ---
		if (!request.isMesh) {

			request.model->Draw(commandList, m_cbvIndex);
		}
		else {
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			request.model->Draw(commandList, request.textureIndex);
		}

		m_cbvIndex++;
	}

	m_drawRequests.clear();
}
#pragma endregion

// --- TUFEngine.cpp ---

#pragma region	各モデルの描画呼び出し

void TUFEngine::DrawSphere(const Vector3& pos, const Vector3& rot, const Vector3& scale, int textureIndex) {
	if (!m_temporarySpheres) {
		m_temporarySpheres = std::make_unique<Sphere>();
		m_temporarySpheres->InitSphere(this);
	}
	if (m_directionalLightResource) {
		m_temporarySpheres->SetLightResource(m_directionalLightResource);
	}

	// 3. 描画リクエストを作成する
	DrawRequest req;
	req.pos = pos;
	req.rot = rot;
	req.scale = scale;
	req.textureIndex = textureIndex;
	req.isMesh = true;

	// 4. コンテナに保存された「絶対に消えない球体」のポインタを安全に取得して登録！
	req.model = m_temporarySpheres.get();

	// 5. リクエストを登録
	m_drawRequests.push_back(req);
}

//三角形
void TUFEngine::DrawTriangle(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color) {
	if (!m_temporaryTriangle) {
		m_temporaryTriangle = std::make_unique<TriangleModel>();
		m_temporaryTriangle->Initialize(this);
	}

	// ← 現在の描画インデックスに頂点を書き込む
	int triIndex = (int)m_drawRequests.size();
	m_temporaryTriangle->UpdateVertices({ -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, triIndex * 3 + 0);
	m_temporaryTriangle->UpdateVertices({ 0.0f,  0.5f, 0.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, triIndex * 3 + 1);
	m_temporaryTriangle->UpdateVertices({ 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, triIndex * 3 + 2);

	DrawRequest req;
	req.model = m_temporaryTriangle.get();
	req.pos = pos;
	req.rot = rot;
	req.scale = scale;
	req.color = color;
	req.isMesh = false;
	m_drawRequests.push_back(req);
}

void TUFEngine::DrawMesh(MeshModel* mesh, Vector3 pos, Vector3 rot, Vector3 scale) {
	if (!mesh) return;

	DrawRequest req;
	req.model = mesh;
	req.pos = pos;
	req.rot = rot;
	req.scale = scale;
	req.textureIndex = mesh->GetTextureIndex(); // ★メンバから自動取得
	req.isMesh = true;
	m_drawRequests.push_back(req);
}

void TUFEngine::DrawMeshTriangle(
	Vector3 v0, Vector3 v1, Vector3 v2,
	Vector4 color,
	std::vector<Vector2> uvs,
	Vector3 rot, Vector3 scale,
	int index)
{
	if (!m_temporaryTriangle) {
		m_temporaryTriangle = std::make_unique<TriangleModel>();
		m_temporaryTriangle->Initialize(this);
	}


	int meshTriangleCount = 0;
	for (const auto& request : m_drawRequests) {
		if (request.model == m_temporaryTriangle.get()) {
			meshTriangleCount++;
		}
	}

	int vertexStartIndex = meshTriangleCount * 3;


	m_temporaryTriangle->UpdateVertices(v0, (uvs.size() > 0 ? uvs[0] : Vector2{ 0.0f, 1.0f }), { 0.0f, 1.0f, 0.0f }, vertexStartIndex + 0);
	m_temporaryTriangle->UpdateVertices(v1, (uvs.size() > 1 ? uvs[1] : Vector2{ 0.5f, 0.0f }), { 0.0f, 1.0f, 0.0f }, vertexStartIndex + 1);
	m_temporaryTriangle->UpdateVertices(v2, (uvs.size() > 2 ? uvs[2] : Vector2{ 1.0f, 1.0f }), { 0.0f, 1.0f, 0.0f }, vertexStartIndex + 2);

	// 4. 描画リクエストを作成してエンジンに注文を登録
	DrawRequest req;
	req.model = m_temporaryTriangle.get();
	req.color = color;


	req.pos = { 0.0f, 0.0f, 0.0f };
	req.rot = { 0.0f, 0.0f, 0.0f };
	req.scale = { 1.0f, 1.0f, 1.0f };
	req.textureIndex = 0; // 必要に応じてテクスチャIDを指定してください
	req.isMesh = true;

	m_drawRequests.push_back(req);
}

void TUFEngine::DrawDynamicMesh(DynamicMesh& mesh, Vector4 color) {
	auto& indices = mesh.getIndices();
	auto& vertices = mesh.getVertices();

	for (int i = 0; i < (int)indices.size(); i += 3) {
		int i0 = indices[i] * 3;
		int i1 = indices[i + 1] * 3;
		int i2 = indices[i + 2] * 3;

		Vector3 v0 = { vertices[i0],     vertices[i0 + 1], vertices[i0 + 2] };
		Vector3 v1 = { vertices[i1],     vertices[i1 + 1], vertices[i1 + 2] };
		Vector3 v2 = { vertices[i2],     vertices[i2 + 1], vertices[i2 + 2] };

		DrawTriangle({}, { 0,0,0 }, { 1,1,1 }, color);
	}
}

void TUFEngine::DrawDynamicMeshWithNormal(
	DynamicMesh& mesh,
	std::vector<Vector4>& colors)
{
	auto& indices = mesh.getIndices();
	auto& vertices = mesh.getVertices();

	if (!m_temporaryTriangle) {
		m_temporaryTriangle = std::make_unique<TriangleModel>();
		m_temporaryTriangle->Initialize(this);
	}

	int triangleCount = 0;

	// グリッドから三角形を1枚ずつ取り出して処理するループ
	for (int i = 0; i < (int)indices.size(); i += 3) {
		int idx0 = indices[i];
		int idx1 = indices[i + 1];
		int idx2 = indices[i + 2];

		// DynamicMeshの頂点バッファ(float配列)からXYZ座標を抽出
		int i0 = idx0 * 3;
		int i1 = idx1 * 3;
		int i2 = idx2 * 3;

		Vector3 v0 = { vertices[i0], vertices[i0 + 1], vertices[i0 + 2] };
		Vector3 v1 = { vertices[i1], vertices[i1 + 1], vertices[i1 + 2] };
		Vector3 v2 = { vertices[i2], vertices[i2 + 1], vertices[i2 + 2] };

		// ⭕【安全対策】colors 配列が頂点数より少ない場合でもクラッシュしないようにガード
		Vector4 c0 = (idx0 < (int)colors.size()) ? colors[idx0] : Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 c1 = (idx1 < (int)colors.size()) ? colors[idx1] : Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 c2 = (idx2 < (int)colors.size()) ? colors[idx2] : Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };

		// 3頂点の平均色を計算
		Vector4 averageColor = {
			(c0.x + c1.x + c2.x) / 3.0f,
			(c0.y + c1.y + c2.y) / 3.0f,
			(c0.z + c1.z + c2.z) / 3.0f,
			1.0f
		};

		// 頂点バッファ上の書き込み開始位置（3頂点ずつ進む）
		int vertexStartIndex = triangleCount * 3;

		// ⭕【クラッシュ防止】TriangleModelの最大頂点数（30000）を超えそうならループを抜ける
		if (vertexStartIndex + 2 >= 30000) {
			break;
		}

		// ⭕【重要】裏返り防止のため、頂点の流し込み順（v1 と v2）を入れ替えてカリングを回避
		m_temporaryTriangle->UpdateVertices(v0, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, vertexStartIndex + 0);
		m_temporaryTriangle->UpdateVertices(v2, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, vertexStartIndex + 1); // ここを入れ替え
		m_temporaryTriangle->UpdateVertices(v1, { 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, vertexStartIndex + 2); // ここを入れ替え

		// 描画リクエストを作成して登録
		DrawRequest req;
		req.model = m_temporaryTriangle.get();
		req.color = averageColor;
		req.pos = { 0.0f, 0.0f, 0.0f }; // 頂点座標自体がすでに波の形なので原点固定
		req.rot = { 0.0f, 0.0f, 0.0f };
		req.scale = { 1.0f, 1.0f, 1.0f };
		req.textureIndex = 0;
		req.isMesh = true;

		m_drawRequests.push_back(req);

		triangleCount++;
	}
}
#pragma endregion