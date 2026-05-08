#include <Windows.h>
#include <cstdint>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>

#pragma comment(lib,"DbgHelp.lib")
#pragma comment(lib,"dxguid.lib")

#include "LogSistem.h"
#include "TUFEngine.h"
#include "VertexResource.h"
#include "Vector.h"


#pragma region dump
// --- クラッシュ時にメモリの状態（ダンプファイル）を保存するための関数群 ---
static int Dump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps//%04d_%02d_%02d_%02d%02d.dmp",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute);

	HANDLE dumpFileHandle = CreateFile(filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		0, CREATE_ALWAYS, 0, 0);

	DWORD processID = GetCurrentProcessId();
	DWORD threadID = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadID;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = true;

	MiniDumpWriteDump(GetCurrentProcess(), processID, dumpFileHandle,
		MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	CloseHandle(dumpFileHandle);
	return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	Dump(exception);
	return EXCEPTION_EXECUTE_HANDLER;
}
#pragma endregion

#ifdef _DEBUG
// --- デバッグ層：エラーがあった時にコンソールに詳細を出してくれる機能 ---
void EnableDebugLayer() {
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
		debugController->Release();
	}
}

// --- メッセージフィルタ：特定の警告や情報を無視する設定 ---
static void SetupInfoQueue(ID3D12Device* device) {
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

// --- メイン関数：ここからプログラムが始まる ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	SetUnhandledExceptionFilter(ExportDump); // クラッシュ時にダンプを出す設定を登録
	InitializeLog(); // ログシステムの初期化

	// --- ウィンドウの設定と登録 ---
	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"MyWindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	const int32_t kClineWidth = 1280;
	const int32_t kClineHeight = 720;

	// クライアント領域のサイズからウィンドウ全体のサイズを計算
	RECT wrc = { 0, 0, kClineWidth, kClineHeight };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウ作成
	HWND hwnd = CreateWindow(
		wc.lpszClassName, L"CG2", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top,
		nullptr, nullptr, wc.hInstance, nullptr
	);

#ifdef _DEBUG
	EnableDebugLayer(); // デバッグレイヤー有効化
#endif

	// --- 自作エンジンの初期化 ---
	TUFEngine* engine = new TUFEngine(kClineWidth, kClineHeight, hwnd);
	ShowWindow(hwnd, nCmdShow);



#ifdef _DEBUG
	SetupInfoQueue(engine->GetDevice()); // エラー情報の取得設定
#endif

	HRESULT hr = S_OK;

	// --- 頂点リソースの作成：三角形の形を作る ---
	ID3D12Resource* vertexResource = CreateVertexResource(engine->GetDevice(), sizeof(Vector4) * 3, hr);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = CreateVertexBufferView(vertexResource, sizeof(Vector4) * 3, sizeof(Vector4));

	// 頂点座標を書き込む（Mapでメモリを繋いで直接代入）
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = { -0.5f, -0.5f, 0.0f, 1.0f }; // 左下
	vertexData[1] = { 0.0f,  0.5f, 0.0f, 1.0f };  // 上
	vertexData[2] = { 0.5f, -0.5f, 0.0f, 1.0f };  // 右下

	// --- マテリアル（色）リソースの作成 ---
	ID3D12Resource* materialResource = CreateBufferResource(engine->GetDevice(), sizeof(Vector4));
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { 1.0f, 0.0f, 0.0f, 1.0f }; // 初期の色は赤

	// --- 行列（トランスフォーム）の初期データ準備 ---
	TransformData transformData{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // オブジェクト用
	TransformData cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} }; // カメラ用
	Matrix4x4 worldMatrix = MakeAffineMatrix(transformData.scale, transformData.rotate, transformData.translate);

	// --- WVP行列リソースの作成：シェーダーに行列を渡すためのバッファ ---
	ID3D12Resource* wvpResource = CreateBufferResource(engine->GetDevice(), sizeof(Matrix4x4));
	Matrix4x4* wvpData = nullptr;
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = MakeIdentity4x4();

	// カメラの配置を決定する行列
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -5.0f }
	);

	// --- メインループ：ここが毎フレーム実行される ---
	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {
			// 1. オブジェクトを回転させる（更新）
			transformData.rotate.y += 0.13f;
			worldMatrix = MakeAffineMatrix(transformData.scale, transformData.rotate, transformData.translate);

			// 2. カメラ行列からビュー行列（カメラの逆の動き）を作成
			Matrix4x4 viwMatrix = Inverse(cameraMatrix);

			// 3. プロジェクション行列（遠近感）を作成
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
				0.45f, static_cast<float>(kClineWidth) / kClineHeight, 0.1f, 100.0f
			);

			// 4. 全部掛け合わせて WVP 行列を完成させる (World -> View -> Projection)
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viwMatrix, projectionMatrix));

			// 5. GPU側のメモリに書き込む
			*wvpData = worldViewProjectionMatrix;

			// --- タイトルバーにデバッグ情報を表示 ---
			std::wstring debugText = std::format(
				L"RotY:{:.2f} | WorldMatrix Row3:[{:.1f}, {:.1f}, {:.1f}, {:.1f}]",
				transformData.rotate.y,
				worldMatrix.m[2][0], worldMatrix.m[2][1], worldMatrix.m[2][2], worldMatrix.m[2][3]
			);
			SetWindowText(hwnd, debugText.c_str());

#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame();

			ImGui::ShowDemoWindow();

			ImGui::Render();

#endif // USE_IMGUI




			// 6. 描画開始処理（コマンドリストのリセットなど）
			engine->PreDraw();

			// 7. GPUへの命令発行
			engine->GetCommandList()->SetGraphicsRootSignature(engine->GetRootSignature());
			engine->GetCommandList()->SetPipelineState(engine->GetPipelineState());
			engine->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
			engine->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// RootParameter[0] に色の情報、[1] に行列の情報をバインド
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

			// 三角形の描画（頂点3つ分）
			engine->GetCommandList()->DrawInstanced(3, 1, 0, 0);

			// 8. 描画終了処理（バッファの入れ替えなど）
			engine->PostDraw();
		}
	}

#ifdef USE_IMGUI
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif // USE_IMGUI

	// --- 解放処理：使ったメモリを返す ---
	vertexResource->Release();
	materialResource->Release();
	delete engine;

	return 0;
}