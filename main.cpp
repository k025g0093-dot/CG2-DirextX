#include "TUFEngine.h"
#include "Sphere.h"



// --- メイン関数：ここからプログラムが始まる ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	SetUnhandledExceptionFilter(ExportDump);

	// 2. エンジンの起動（この中で窓も作られる）
	const int32_t kClineWidth = 1280;
	const int32_t kClineHeight = 720;

	TUFEngine* engine = new TUFEngine(kClineWidth, kClineHeight, L"TUFEngine");
	assert(engine->GetDevice() != nullptr);

	TextureManager* textureManager = TextureManager::GetInstance();

	textureManager->Initialize(
		engine->GetDevice(),
		engine->GetSrvDescriptorHeap(),
		engine->GetCommandList()
	);
	// 3. 窓の表示
	ShowWindow(engine->GetHwnd(), nCmdShow);

	// リソース読み込み
	int uvChecker = textureManager->LoadTexture("resources/uvChecker.png");
	int monsterBall = textureManager->LoadTexture("resources/monsterBall.png");

	HRESULT hr = S_OK;

	uint32_t sphereVertexCount = 16 * 16 * 6;


	// --- 頂点リソースの作成：三角形の形を作る ---
// サイズもstrideもVertexDataに合わせる
	ID3D12Resource* vertexResource = CreateVertexResource(
		engine->GetDevice(), sizeof(VertexData) * sphereVertexCount, hr);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = CreateVertexBufferView(
		vertexResource, sizeof(VertexData) * sphereVertexCount, sizeof(VertexData)); // ← VertexDataに変更

	ID3D12Resource* vertexResourceSprite = CreateBufferResource(
		engine->GetDevice(), sizeof(VertexData) * sphereVertexCount
	);


	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	// ① まずMapしてアドレスを取得
	VertexData* vertexData = {};
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// ② 取得したアドレスにデータを書き込む
	UpdateSphere(vertexData);

	// ③ 書き終わったらUnmap
	vertexResource->Unmap(0, nullptr);


	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	vertexDataSprite[0] = { {0.0f, 360.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }; // 左下
	vertexDataSprite[0].normal = { 0.0f,-1.0f,0.0f };
	vertexDataSprite[1] = { {0.0f,0.0f,0.0f,1.0f},{0.0f,0.0f} };//左上
	vertexDataSprite[2] = { {640.0f,360.0f,0.0f,1.0f},{1.0f,1.0f} };//右下

	vertexDataSprite[3] = { {0.0f,0.0f,0.0f,1.0f},{0.0f,0.0f} };//左上
	vertexDataSprite[4] = { {640.0f,0.0f,0.0f,1.0f},{1.0f,0.0f} };//右上
	vertexDataSprite[5] = { {640.0f,360.0f,0.0f,1.0f},{1.0f,1.0f} };//右下
	vertexResourceSprite->Unmap(0, nullptr); // 書き終わったら Unmap するのが安全


	// --- マテリアル（色）リソースの作成 ---
	ID3D12Resource* materialResource = CreateBufferResource(engine->GetDevice(), sizeof(Material));
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色は赤



	ID3D12Resource* materialResourceSprite = CreateBufferResource(engine->GetDevice(), sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色は赤
	materialDataSprite->enableLifhting = false;

	materialData->enableLifhting = 0;


	// --- 行列（トランスフォーム）の初期データ準備 ---
	TransformData transformData{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // オブジェクト用
	TransformData cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} }; // カメラ用
	Matrix4x4 worldMatrix = MakeAffineMatrix(transformData.scale, transformData.rotate, transformData.translate);

	// --- WVP行列リソースの作成：シェーダーに行列を渡すためのバッファ ---
	ID3D12Resource* wvpResource = CreateBufferResource(engine->GetDevice(), sizeof(TransformationMatrix));
	TransformationMatrix* wvpData = nullptr; // 型を Matrix4x4 から変更
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));

	// 初期化（WVPとWorld両方に単位行列を入れる）
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();


	//transformSprite用のリソース作成
	ID3D12Resource* transformationMatrixResourceSprite =
		CreateBufferResource(engine->GetDevice(), sizeof(TransformationMatrix));
	Matrix4x4* transformationMatrixDataSprite = nullptr;
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	*transformationMatrixDataSprite = MakeIdentity4x4();
	TransformData transformDataSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };


	// カメラの配置を決定する行列
	Matrix4x4 cameraMatrix = MakeAffineMatrix(
		{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -10.0f }
	);

	ID3D12Resource* directionalLightDataResource =
		CreateBufferResource(engine->GetDevice(), sizeof(DirectionalLLight));

	DirectionalLLight* directionalLightData = nullptr;


	directionalLightDataResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// 4. データの中身を書き込む
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1;
	//directionalLightDataResource->Unmap(0, nullptr);

	bool useMonsterBall = true;

	// --- メインループ：ここが毎フレーム実行される ---
	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->OnUpdate();

			// 1. オブジェクトを回転させる（更新）
			transformData.rotate.y += 0.01f;

			cameraMatrix = MakeAffineMatrix(
				cameraTransform.scale,
				cameraTransform.rotate,
				cameraTransform.translate
			);

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
			wvpData->WVP = worldViewProjectionMatrix; // 1枚目
			wvpData->World = worldMatrix;


			//スプライト用のWVPを作成
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(
				transformDataSprite.scale,
				transformDataSprite.rotate,
				transformDataSprite.translate
			);

			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(
				0.0f, 0.0f,
				static_cast<float>(kClineWidth),
				static_cast<float>(kClineHeight),
				0.1f, 100.0f
			);

			Matrix4x4 worldViewProjectSprite = Multiply(
				worldMatrixSprite, Multiply(
					viewMatrixSprite, projectionMatrixSprite
				)
			);

			*transformationMatrixDataSprite = worldViewProjectSprite;


#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");

			// --- スプライト設定 ---
			if (ImGui::CollapsingHeader("Sprite Settings")) {
				ImGui::DragFloat2("Sprite Position", &transformDataSprite.translate.x, 1.0f, 0.0f, 1280.0f);
				ImGui::DragFloat3("Sprite Scale", &transformDataSprite.scale.x, 0.1f, 0.1f, 10.0f);
				ImGui::DragFloat3("Sprite Rotate", &transformDataSprite.rotate.z, 0.01f);
				ImGui::Checkbox("useMonsterBall", &useMonsterBall);
			}

			// ---  カメラ設定 (ここを追加) ---
			if (ImGui::CollapsingHeader("Camera Settings")) {
				// ※ お使いのカメラの変数名（例: cameraTransform など）に適宜書き換えてください
				ImGui::DragFloat3("Camera Position", &cameraTransform.translate.x, 0.1f);
				ImGui::DragFloat3("Camera Rotation", &cameraTransform.rotate.x, 0.01f);

				// 必要に応じて、画角(FOV)や注視点(LookAt)などもここに追加できます
			}

			// --- ⭕ ライティング設定 ---
			if (directionalLightData && ImGui::CollapsingHeader("Lighting Settings")) {

				// 改善ポイント①：UI操作専用の「一時変数」を用意する
				static float lightDir[3] = { 1.0f, -1.0f, 1.0f };

				// 一時変数をスライダーで操作する
				ImGui::ColorEdit4("Light Color", &directionalLightData->color.x);
				ImGui::DragFloat3("Light Direction", lightDir, 0.01f, -1.0f, 1.0f);
				ImGui::DragFloat("Light Intensity", &directionalLightData->intensity, 0.01f, 0.0f, 10.0f);
				ImGui::DragInt("materialData->enableLifhting ", &materialData->enableLifhting, 1, 0);

				// 改善ポイント②：スライダーの値を計算（正規化）してから、GPU用のデータに流し込む
				float length = sqrtf(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
				if (length > 0.0f) {
					directionalLightData->direction.x = lightDir[0] / length;
					directionalLightData->direction.y = lightDir[1] / length;
					directionalLightData->direction.z = lightDir[2] / length;
				}


			}

			ImGui::End();
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
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightDataResource->GetGPUVirtualAddress());

			engine->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager->GetGPUHandle(useMonsterBall ? monsterBall : uvChecker));


			engine->GetCommandList()->DrawInstanced(sphereVertexCount, 1, 0, 0);

			engine->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			engine->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager->GetGPUHandle(uvChecker));

			engine->GetCommandList()->DrawInstanced(6, 1, 0, 0);

			// 8. 描画終了処理（バッファの入れ替えなど）
			engine->PostDraw();
		}
		if (Input::GetKeyDown(VK_ESCAPE)) break;

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
