#include "TUFEngine.h"
#include "Sphere.h"
#include "WaveGrid.h"
#include <algorithm>

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

	MeshModel* modelData = engine->LoadModel("resources", "plane.obj");


	HRESULT hr = S_OK;

	uint32_t sphereVertexCount = 16 * 16 * 6;


	// --- 頂点リソースの作成：三角形の形を作る ---
// サイズもstrideもVertexDataに合わせる
	
#pragma region	リソースたちこれもたくさんあるので割愛
	
	ID3D12Resource* vertexResource = CreateVertexResource(
		engine->GetDevice(), sizeof(VertexData) * sphereVertexCount, hr);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = CreateVertexBufferView(
		vertexResource, sizeof(VertexData) * sphereVertexCount, sizeof(VertexData)); // ← VertexDataに変更

	ID3D12Resource* vertexResourceSprite = CreateBufferResource(
		engine->GetDevice(), sizeof(VertexData) * sphereVertexCount
	);

	ID3D12Resource* indexResourceSprite = CreateBufferResource(
		engine->GetDevice(), sizeof(uint32_t) * 6
	);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSpraite{};
	indexBufferViewSpraite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	indexBufferViewSpraite.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewSpraite.Format = DXGI_FORMAT_R32_UINT;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);



	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	vertexDataSprite[0] = { {0.0f, 360.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }; // 左下
	vertexDataSprite[0].normal = { 0.0f,-1.0f,0.0f };
	vertexDataSprite[1] = { {0.0f,0.0f,0.0f,1.0f},{0.0f,0.0f} };//左上
	vertexDataSprite[2] = { {640.0f,360.0f,0.0f,1.0f},{1.0f,1.0f} };//右下

	vertexDataSprite[3] = { {640.0f,0.0f,0.0f,1.0f},{1.0f,0.0f} };//右上
	vertexResourceSprite->Unmap(0, nullptr); // 書き終わったら Unmap するのが安全

	uint32_t* indexDataSpraite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSpraite));
	indexDataSpraite[0] = 0; indexDataSpraite[1] = 1; indexDataSpraite[2] = 2;
	indexDataSpraite[3] = 1; indexDataSpraite[4] = 3; indexDataSpraite[5] = 2;


	// --- マテリアル（色）リソースの作成 ---
	ID3D12Resource* materialResource = CreateBufferResource(engine->GetDevice(), sizeof(Material));
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色は赤
	materialData->uvTransform = MakeIdentity4x4();


	ID3D12Resource* materialResourceSprite = CreateBufferResource(engine->GetDevice(), sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 初期の色は赤
	materialDataSprite->enableLifhting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();
	materialData->enableLifhting = 0;

	TransformData uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};


	// --- 行列（トランスフォーム）の初期データ準備 ---
	TransformData transformData{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // オブジェクト用
	TransformData cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} }; // カメラ用
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
	engine->SetDirectionalLightResource(directionalLightDataResource);
	//directionalLightDataResource->Unmap(0, nullptr);
#pragma endregion
	
		// グリッド設定
	const int cubeCountX = 10;
	const int cubeCountZ = 10;
	float cubeSize = 1.0f;
	float spacing = 1.0f;

	// WaveGrid初期化
	WaveGrid waveGrid(cubeCountX, cubeCountZ);

	// 壁の設定
	int wallX = cubeCountX / 3;
	int holeStart = cubeCountZ / 2 - 3;
	int holeEnd = cubeCountZ / 2 + 3;
	for (int gz = 0; gz < cubeCountZ; gz++) {
		bool isWall = (gz < holeStart || gz >= holeEnd);
		waveGrid.setWall(wallX, gz, isWall);
	}

	float waveStrength = 10.0f;
	float baseDepth = 3.0f;
	bool  showNormal = false;  // 法線表示のON/OFF

	DynamicMesh mesh(cubeCountX, cubeCountZ);
	// ループの外で宣言
	std::vector<Vector4> normalColors(cubeCountX * cubeCountZ);
	float t = 0.0f;


	bool useMonsterBall = true;

	float cameraMoveSpeed = 0.01f;                  // 移動速度
	float cameraRotateSpeed = 0.01f;                // 回転速度

	float rotX = 0.01f;

	// --- メインループ：ここが毎フレーム実行される ---
	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->OnUpdate();

#pragma region カメラと行列の計算長いのでまとめる

			cameraTransform.rotate.y += Input::GetRightStickX() * cameraRotateSpeed;
			cameraTransform.rotate.x -= Input::GetRightStickY() * cameraRotateSpeed;

			cameraTransform.translate.x += Input::GetLeftStickX();
			cameraTransform.translate.z += Input::GetLeftStickY();

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
			engine->SetViewProjectionMatrix(Multiply(viwMatrix, projectionMatrix));
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

			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;
#pragma endregion

#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

			// ====================================================
			// 1. スプライト設定用のウィンドウ
			// ====================================================
			ImGui::Begin("Sprite Matrix"); // ★新しいウィンドウを開始

			ImGui::DragFloat2("Sprite Position", &transformDataSprite.translate.x, 1.0f, 0.0f, 1280.0f);
			ImGui::DragFloat3("Sprite Scale", &transformDataSprite.scale.x, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat3("Sprite Rotate", &transformDataSprite.rotate.z, 0.01f);
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);
			ImGui::SliderAngle("rotX", &rotX);

			ImGui::Separator(); // ちょっと区切り線

			ImGui::DragFloat2("uvTransform Position", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("uvTransform Scale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("uvTransform Rotate", &uvTransformSprite.rotate.z);

			ImGui::End(); // ★スプライトウィンドウの終わり


			// ====================================================
			// 2. ライティング設定用のウィンドウ
			// ====================================================
			if (directionalLightData) {
				ImGui::Begin("Lighting Control"); // ★新しいウィンドウを開始

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

				ImGui::End(); // ★ライティングウィンドウの終わり
			}


			// ====================================================
			// 3. カメラ設定用のウィンドウ
			// ====================================================
			ImGui::Begin("Camera Monitor"); // ★新しいウィンドウを開始

			// カメラの位置を操作・確認 (X, Y, Z)
			ImGui::DragFloat3("Camera Position", &cameraTransform.translate.x, 0.1f);

			// カメラの回転角を操作・確認 (X, Y, Z)
			ImGui::DragFloat3("Camera Rotation", &cameraTransform.rotate.x, 0.01f);

			// 【便利機能】カメラの位置や向きが迷子になったときのリセットボタン
			if (ImGui::Button("Reset Camera")) {
				cameraTransform.translate = { 0.0f, 0.0f, -5.0f };
				cameraTransform.rotate = { 0.0f, 0.0f, 0.0f };
			}

			// 現在の数値をテキストとして小さく表示（デバッグ情報の確認用）
			ImGui::Separator();
			ImGui::Text("Debug Info:");
			ImGui::Text("Pos: X:%.2f, Y:%.2f, Z:%.2f", cameraTransform.translate.x, cameraTransform.translate.y, cameraTransform.translate.z);
			ImGui::Text("Rot: X:%.2f, Y:%.2f, Z:%.2f", cameraTransform.rotate.x, cameraTransform.rotate.y, cameraTransform.rotate.z);

			ImGui::End(); // ★カメラウィンドウの終わり

			ImGui::Begin("Yamato Debug");
			ImGui::SliderFloat("waveStrength", &waveStrength, 0.1f, 20.0f);
			ImGui::SliderFloat("baseDepth", &baseDepth, 1.0f, 10.0f);
			ImGui::Checkbox("showNormal", &showNormal);  // 法線表示切り替え
			// ImGui::Begin の中に追加
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("Triangles: %d", (int)mesh.getIndices().size() / 3);
			if (ImGui::Button("Reset")) {
				waveGrid.reset();
				t = 0.0f;
			}
			ImGui::End();
			
			ImGui::Render();
#endif // USE_IMGUI

			t += 0.016f;
			for (int gz = 1; gz < cubeCountZ - 1; gz++) {
				waveGrid.mCurrent[waveGrid.valueIndex(1, gz)] = sinf(t * 3.0f) * waveStrength;
			}
			//rotX += 0.001f;
			waveGrid.update();

			// 6. 描画開始処理（コマンドリストのリセットなど）
			engine->PreDraw();

			for (int i = 0; i < 2; ++i) {
				engine->DrawSphere({ 0.0f + i * 5.0f, 0.0f, 5.0f }, { 0.0f,  rotX, 0.0f }, { 1.0f, 1.0f, 1.0f }, useMonsterBall ? monsterBall : uvChecker);
			}

			engine->DrawMesh(
				modelData,
				{ 0.0f, 0.0f, 0.0f },    // 位置 (とりあえず原点)
				{ 0.0f, rotX, 0.0f },    // 回転
				{ 1.0f, 1.0f, 1.0f }
			);

			for (int i = 0; i < 10; i++) {
				
				engine->DrawTriangle(
					{ 0.0f+i*0.5f, 2.0f, 0.0f },    // 位置 (とりあえず原点)
					{ 0.0f, rotX, 0.0f },    // 回転
					{ 1.0f, 1.0f, 1.0f },
					{ 1,1,1,1 }
				);
			}
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
			engine->GetCommandList()->IASetIndexBuffer(&indexBufferViewSpraite);
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			engine->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			engine->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager->GetGPUHandle(uvChecker));

			engine->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);


			for (int iz = 0; iz < cubeCountZ; iz++) {
				for (int ix = 0; ix < cubeCountX; ix++) {
					float h = waveGrid.getHeight(ix, iz);
					mesh.updateHeight(ix, iz, h);

					auto n = waveGrid.getNormal(ix, iz);
					mesh.updateNormal(ix, iz, n.x, n.y, n.z);

					// 法線をRGBに変換
					int idx = iz * cubeCountX + ix;
					normalColors[idx] = {
						(n.x + 1.0f) / 2.0f,
						(n.y + 1.0f) / 2.0f,
						(n.z + 1.0f) / 2.0f,
						1.0f
					};
				}
			}

			//// 描画
			engine->DrawDynamicMeshWithNormal(mesh, normalColors);


			// 8. 描画終了処理（バッファの入れ替えなど）
			engine->PostDraw();
		}

		//ESCでゲームを強制終了
#ifdef _DEBUG
		//デバック時のみ有効
		if (Input::GetKeyDown(VK_ESCAPE)) break;
#endif // DEBUG


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
