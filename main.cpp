#include "TUFEngine.h"
#include "ModelManager.h"
#include "Sphere.h"
#include "WaveGrid.h"
#include "Camera.h"
#include <algorithm>

#include <fstream>
#include <iomanip>

#include "DebugCamer.h"
#include "Sound.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	SetUnhandledExceptionFilter(ExportDump);



	const int32_t kClineWidth = 1280;
	const int32_t kClineHeight = 720;


	TUFEngine* engine = new TUFEngine(kClineWidth, kClineHeight, L"CG2_TUFEngine_LE2B_29_ヤマト_ユウヤ");
	assert(engine->GetDevice() != nullptr);

	ShowWindow(engine->GetHwnd(), nCmdShow);

	int uvChecker = engine->LoadTexture("resources/uvChecker.png");
	int monsterBall = engine->LoadTexture("resources/monsterBall.png");
	int umi = engine->LoadTexture("resources/ao.jpg");

	int normal = engine->LoadTexture("resources/top normal.png");

	MeshModel* modelData = ModelManager::GetInstance()->LoadModel("resources/skyDome", "sky_sphere.obj");
	if (modelData) modelData->SetEnableLighting(0);

	Sound* sound = new Sound;
	SoundData soundData1 = sound->SoundLoad("resources/fanfare.wav");
	SoundData title = sound->SoundLoad("resources/title.mp3");

	DebugCamer* debugCamer_ = new DebugCamer;
	debugCamer_->Initialize((float)kClineWidth, (float)kClineHeight);

	const int cubeCountX = 200;
	const int cubeCountZ = 200;

	WaveGrid waveGrid(cubeCountX, cubeCountZ, ModelManager::GetInstance()->GetSceneObjects());

	// ========== GPU初期化 ==========
	waveGrid.InitializeGPU(engine->GetDevice(), engine);

	int wallX = cubeCountX / 5;
	int wall2 = cubeCountX / 2;
	int holeStart = cubeCountZ / 2 - 3;
	int holeEnd = cubeCountZ / 2 + 3;

	//for (int gz = 0; gz < cubeCountZ; gz++) {
	//	waveGrid.setWall(wallX, gz, (gz < holeStart || gz >= holeEnd));
	//	waveGrid.setWall(wall2, gz, (gz < holeStart || gz >= holeEnd));
	//}

	float waveStrength = 10.0f;
	DynamicMesh mesh(cubeCountX, cubeCountZ);
	std::vector<Vector4> normalColors(cubeCountX * cubeCountZ);
	float t = 0.0f;

	const auto& droppedMeshes = ModelManager::GetInstance()->GetSceneObjects();

	engine->m_camera.transform.translate.x = 0.0f;
	engine->m_camera.transform.translate.y = 200.0f;
	engine->m_camera.transform.translate.z = -300.0f;
	engine->m_camera.transform.rotate.x = 0.6f;

	bool  useMonsterBall = true;
	float cameraRotateSpeed = 0.016f;

	Vector3 spherePos[2] = { {0.0f, 0.0f, 5.0f}, {5.0f, 0.0f, 5.0f} };
	Vector3 sphereRot[2] = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Vector3 sphereScale[2] = { {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} };
	bool    sphereUseMonsterBall[2] = { true, true };

	Vector3 meshPos = { 0.0f, 0.0f, 0.0f };
	Vector3 meshRot = { 0.0f, 0.0f, 0.0f };
	Vector3 meshScale = { 1.0f, 1.0f, 1.0f };

	Vector3 triBasePos = { 0.0f, 2.0f, 0.0f };
	Vector3 triRot = { 0.0f, 0.0f, 0.0f };
	Vector3 triScale = { 1.0f, 1.0f, 1.0f };
	Vector4 triColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	Vector2 spritePos = { 0.0f, 0.0f };
	Vector2 spriteScale = { 1.0f, 1.0f };
	Vector3 spriteRot = { 0.0f, 0.0f, 0.0f };

	TransformData uvTransformSprite{};
	uvTransformSprite.scale = { 1.0f, 1.0f, 1.0f };
	uvTransformSprite.rotate = { 0.0f, 0.0f, 0.0f };
	uvTransformSprite.translate = { 0.0f, 0.0f, 0.0f };

	int frameIndex = 0;
	int observeX = wallX + 40;

	LightManager::GetInstance()->SetPerObjectLight(1, { {1,1,1,1}, {0,-1,0}, 1.0f });
	LightManager::GetInstance()->SetPerObjectLight(2, { {1,1,1,1}, {0,-1,0}, 1.0f });

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			////================================================================================================================
			////更新処理ここから
			////================================================================================================================

			engine->OnUpdate();

#ifdef USE_IMGUI
			engine->GetImGuiManager()->onDrawGUI = [&]() {

				if (ImGui::Begin("SceneSettingsModel")) {

					if (ImGui::CollapsingHeader("Wave Settings")) {
						ImGui::DragFloat("Wave Strength", &waveStrength, 0.1f, 0.0f, 50.0f);
					}

					if (ImGui::CollapsingHeader("Sphere Settings")) {
						for (int i = 0; i < 2; ++i) {
							int lightId = i + 1;
							ImGui::PushID(i);
							ImGui::Text("Sphere %d", i);
							ImGui::DragFloat3("Position", &spherePos[i].x, 0.1f);
							ImGui::DragFloat3("Rotation", &sphereRot[i].x, 0.01f);
							ImGui::DragFloat3("Scale", &sphereScale[i].x, 0.01f, 0.01f, 10.0f);
							ImGui::Checkbox("MonsterBall", &sphereUseMonsterBall[i]);

							ImGui::SeparatorText("Light");
							DirectionalLight light = LightManager::GetInstance()->GetPerObjectLight(lightId);
							bool changed = false;
							changed |= ImGui::ColorEdit4("##Color", &light.color.x);
							changed |= ImGui::DragFloat3("##Direction", &light.direction.x, 0.01f, -1.0f, 1.0f);
							changed |= ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 10.0f);

							if (changed) LightManager::GetInstance()->SetPerObjectLight(lightId, light);

							ImGui::Separator();
							ImGui::PopID();
						}
					}

					if (ImGui::CollapsingHeader("Mesh Settings")) {
						ImGui::DragFloat3("Position", &meshPos.x, 0.1f);
						ImGui::DragFloat3("Rotation", &meshRot.x, 0.01f);
						ImGui::DragFloat3("Scale", &meshScale.x, 0.01f, 0.01f, 10.0f);
					}

					if (ImGui::CollapsingHeader("Triangle Settings")) {
						ImGui::DragFloat3("Base Position", &triBasePos.x, 0.1f);
						ImGui::DragFloat3("Rotation", &triRot.x, 0.01f);
						ImGui::DragFloat3("Scale", &triScale.x, 0.01f, 0.01f, 10.0f);
						ImGui::ColorEdit4("Color", &triColor.x);
					}

					if (ImGui::CollapsingHeader("Sprite Settings")) {
						ImGui::DragFloat2("Position", &spritePos.x, 1.0f, -1280.0f, 1280.0f);
						ImGui::DragFloat2("Scale", &spriteScale.x, 0.1f, 0.01f, 20.0f);
						ImGui::SliderAngle("Rotate", &spriteRot.z);
					}

					if (ImGui::CollapsingHeader("Sprite UV Settings")) {
						ImGui::DragFloat2("UV Translate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
						ImGui::DragFloat2("UV Scale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
						ImGui::SliderAngle("UV Rotate", &uvTransformSprite.rotate.z);
					}
					ImGui::End();
				}
				};
#endif // USE_IMGUI

			engine->m_camera.transform.rotate.y += Input::GetRightStickX() * cameraRotateSpeed;
			engine->m_camera.transform.rotate.x -= Input::GetRightStickY() * cameraRotateSpeed;
			engine->m_camera.transform.translate.x += Input::GetLeftStickX();
			engine->m_camera.transform.translate.z += Input::GetLeftStickY();

			debugCamer_->Update();

			if (debugCamer_->IsDebug()) {
				engine->SetViewProjectionMatrix(debugCamer_->GetViewProjectionMatrix());
			}
			else {
				engine->SetViewProjectionMatrix(
					engine->m_camera.GetViewProjectionMatrix(kClineWidth, kClineHeight)
				);
			}

			////// ========== GPU ベースの波シミュレーション ==========
			//t += 0.01f;
			//waveGrid.DispatchWaveSimulation(t, 30.0f, waveStrength);
			//waveGrid.ReadbackToCPU();
			//waveGrid.setObjectWall(ModelManager::GetInstance()->GetSceneObjects());

			frameIndex++;

			////================================================================================================================
			////描画処理ここから
			////================================================================================================================

			engine->PreDraw();

			//{
			//	// DynamicMeshModel が未初期化なら先に作らせる
			//	if (!engine->GetDynamicMeshModel())
			//		engine->DrawDynamicMeshWithNormal(mesh, normalColors, umi);
			//	VertexData* vb = engine->GetDynamicMeshModel()->GetMappedVertexBuffer();
			//	for (int iz = 0; iz < cubeCountZ; iz++) {
			//		for (int ix = 0; ix < cubeCountX; ix++) {
			//			int idx = iz * cubeCountX + ix;
			//			float h = waveGrid.GetHeightFromCache(ix, iz);
			//			Vector4 n = waveGrid.GetNormalFromCache(ix, iz);
			//			vb[idx].position.y = h;
			//			vb[idx].normal = { n.x, n.y, n.z };
			//			normalColors[idx] = {
			//				(n.x + 1.0f) / 2.0f,
			//				(n.y + 1.0f) / 2.0f,
			//				(n.z + 1.0f) / 2.0f,
			//				1.0f
			//			};
			//		}
			//	}
			//}

			//engine->DrawDynamicMeshWithNormal(mesh, normalColors, umi);

			for (int i = 0; i < 2; ++i) {
				engine->DrawSphere(
					spherePos[i],
					sphereRot[i],
					sphereScale[i],
					sphereUseMonsterBall[i] ? monsterBall : uvChecker,
					i + 1
				);
			}

			engine->DrawMesh(modelData, meshPos, meshRot, meshScale, -1);

			for (int i = 0; i < 10; i++) {
				engine->DrawTriangle(
					{ triBasePos.x + i * 0.5f, triBasePos.y, triBasePos.z },
					triRot,
					triScale,
					triColor,
					uvChecker);
			}

			engine->PostDraw();

			////================================================================================================================
			////描画処理ここまで
			////================================================================================================================

			if (Input::GetKeyDown(VK_SPACE)) {
				sound->SoundPlayer(soundData1);
				sound->SoundPlayer(title);
			}
		}

		if (Input::GetKeyDown(VK_ESCAPE)) break;
	}

	sound->SoundUnLoad(&soundData1);
	delete sound;
	delete debugCamer_;
	delete engine;

	return 0;
}