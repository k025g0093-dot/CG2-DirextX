#include "TUFEngine.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord; // テクスチャのどこを使うかの指定
};

// --- メイン関数：ここからプログラムが始まる ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	SetUnhandledExceptionFilter(ExportDump);

	// 2. エンジンの起動（この中で窓も作られる）
	const int32_t kClineWidth = 1280;
	const int32_t kClineHeight = 720;
	TUFEngine* engine = new TUFEngine(kClineWidth, kClineHeight, L"TUFEngine");

	// 3. 窓の表示
	ShowWindow(engine->GetHwnd(), nCmdShow);

	// リソース読み込み
	ID3D12Resource* myTexture = engine->LoadTexture("resources/uvChecker.png");

	HRESULT hr = S_OK;

	// --- 頂点リソースの作成：三角形の形を作る ---
// サイズもstrideもVertexDataに合わせる
	ID3D12Resource* vertexResource = CreateVertexResource(
		engine->GetDevice(), sizeof(VertexData) * 6, hr);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = CreateVertexBufferView(
		vertexResource, sizeof(VertexData) * 6, sizeof(VertexData)); // ← VertexDataに変更


	// ③ データ書き込み（Map）
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = { {-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f} }; // 左下
	vertexData[1] = { { 0.0f,  0.5f, 0.0f, 1.0f}, {0.5f, 0.0f} }; // 上
	vertexData[2] = { { 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f} }; // 右下
	vertexData[3] = { {-0.5f, -0.5f, 0.5f, 1.0f}, {0.0f, 1.0f} }; // 左下
	vertexData[4] = { { 0.0f,  0.0f, 0.0f, 1.0f}, {0.5f, 0.0f} }; // 右上
	vertexData[5] = { { 0.5f,  -0.5f,-0.5f, 1.0f}, {1.0f, 1.0f} }; // 上
	vertexResource->Unmap(0, nullptr); // 書き終わったら Unmap するのが安全


	// --- マテリアル（色）リソースの作成 ---
	ID3D12Resource* materialResource = CreateBufferResource(engine->GetDevice(), sizeof(Vector4));
	Vector4* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色は赤

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
			transformData.rotate.y += 0.01f;
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
			engine->GetCommandList()->SetGraphicsRootDescriptorTable(2, engine->GetTextureSrvHandleGPU());

			// 三角形の描画（頂点3つ分）
			engine->GetCommandList()->DrawInstanced(6, 1, 0, 0);

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
