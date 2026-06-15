#include "TUFEngine.h"
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


	MeshModel* modelData = engine->LoadModel("resources", "axis.obj");

	Sound* sound = new Sound;
	SoundData soundData1 = sound->SoundLoad("resources/fanfare.wav");
	SoundData title = sound->SoundLoad("resources/title.mp3");

	DebugCamer* debugCamer_ = new DebugCamer;
	debugCamer_->Initialize((float)kClineWidth, (float)kClineHeight);


	const int cubeCountX = 200;
	const int cubeCountZ = 200;

	WaveGrid waveGrid(cubeCountX, cubeCountZ, engine->GetDroppedMeshes());

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
#pragma endregion

	const auto& droppedMeshes = engine->GetDroppedMeshes();

	engine->m_camera.transform.translate.x = 0.0f;
	engine->m_camera.transform.translate.y = 200.0f;
	engine->m_camera.transform.translate.z = -300.0f;
	engine->m_camera.transform.rotate.x = 0.6f;

	bool  useMonsterBall = true;
	float cameraRotateSpeed = 0.01f;

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

	//std::ofstream waveLog("wave_analysis.csv");
	//waveLog << "frame,z,height,intensity\n";
	int frameIndex = 0;
	int observeX = wallX + 40;


	LightManager::GetInstance()->SetPerObjectLight(0, { {1,1,1,1}, {0,-1,0}, 1.0f });
	LightManager::GetInstance()->SetPerObjectLight(1, { {1,1,1,1}, {0,-1,0}, 1.0f });


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
							ImGui::PushID(i);
							ImGui::Text("Sphere %d", i);
							ImGui::DragFloat3("Position", &spherePos[i].x, 0.1f);
							ImGui::DragFloat3("Rotation", &sphereRot[i].x, 0.01f);
							ImGui::DragFloat3("Scale", &sphereScale[i].x, 0.01f, 0.01f, 10.0f);
							ImGui::Checkbox("MonsterBall", &sphereUseMonsterBall[i]);

							ImGui::SeparatorText("Light");
							DirectionalLight light = LightManager::GetInstance()->GetPerObjectLight(i);
							bool changed = false;
							changed |= ImGui::ColorEdit4("##Color", &light.color.x);
							changed |= ImGui::DragFloat3("##Direction", &light.direction.x, 0.01f, -1.0f, 1.0f);
							changed |= ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 10.0f);

							if (changed) LightManager::GetInstance()->SetPerObjectLight(i, light);

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

			t += 0.016f;
			for (int gz = 1; gz < cubeCountZ - 1; gz++) {
				waveGrid.mCurrent[waveGrid.valueIndex(1, gz)] = sinf(t * 30.0f) * waveStrength;
			}

			//// 右端から別の波
			//for (int gz = 1; gz < cubeCountZ - 1; gz++) {
			//	waveGrid.mCurrent[waveGrid.valueIndex(cubeCountX - 2, gz)] = sinf(t * 20.0f) * waveStrength;
			//}

			//// 中心点から円形に広がる波
			//waveGrid.addSource(cubeCountX / 2, cubeCountZ / 2, sinf(t * 15.0f) * waveStrength);

			//// 上端から
			//for (int gx = 1; gx < cubeCountX - 1; gx++) {
			//	waveGrid.mCurrent[waveGrid.valueIndex(gx, 1)] = sinf(t * 25.0f + gx * 0.3f) * waveStrength;
			//}
			waveGrid.update();

			waveGrid.setObjectWall(engine->GetDroppedMeshes());

			//if (frameIndex % 60 == 0) {
			//	waveLog << frameIndex;
			//	for (int iz = 0; iz < cubeCountZ; iz++) {
			//		for (int ix = 0; ix < cubeCountX; ix++) {
			//			waveLog << "," << waveGrid.getHeight(ix, iz);
			//		}
			//	}
			//	waveLog << "\n";
			//}
			frameIndex++;

			engine->SetSpriteUVScale({ uvTransformSprite.scale.x, uvTransformSprite.scale.y });
			engine->SetSpriteUVTranslate({ uvTransformSprite.translate.x, uvTransformSprite.translate.y });

			engine->SetSpriteUVRotate(uvTransformSprite.rotate.z);

			////================================================================================================================
			////更新処理ここまで
			////================================================================================================================



			////================================================================================================================
			////描画処理ここから
			////================================================================================================================

			engine->PreDraw();

			for (int i = 0; i < 2; ++i) {
				engine->DrawSphere(
					spherePos[i],
					sphereRot[i],
					sphereScale[i],
					sphereUseMonsterBall[i] ? monsterBall : uvChecker,
					i
					);
			}

			engine->DrawMesh(modelData, meshPos, meshRot, meshScale);



			for (int i = 0; i < 10; i++) {
				engine->DrawTriangle(
					{ triBasePos.x + i * 0.5f, triBasePos.y, triBasePos.z },
					triRot,
					triScale,
					triColor,
					uvChecker);
			}

			//engine->DrawSprite(
			//	spritePos,
			//	720 * spriteScale.x, 360 * spriteScale.y,
			//	spriteRot,
			//	{ 1, 1, 1 },
			//	{ 1, 1, 1, 1 },
			//	uvChecker);

			for (int iz = 0; iz < cubeCountZ; iz++) {
				for (int ix = 0; ix < cubeCountX; ix++) {
					float h = waveGrid.getHeight(ix, iz);
					mesh.updateHeight(ix, iz, h);
					auto  n = waveGrid.getNormal(ix, iz);
					mesh.updateNormal(ix, iz, n.x, n.y, n.z);
					int idx = iz * cubeCountX + ix;
					normalColors[idx] = {
						(n.x + 1.0f) / 2.0f,
						(n.y + 1.0f) / 2.0f,
						(n.z + 1.0f) / 2.0f,
						1.0f
					};
				}
			}


			//engine->DrawDynamicMeshWithNormal(mesh, normalColors, umi);

			engine->PostDraw();

			////================================================================================================================
			////描画処理ここまで
			////================================================================================================================

			//謎の音声鳴らす処理多分ここじゃないほうがいい
			if (Input::GetKeyDown(VK_SPACE)) {
				sound->SoundPlayer(soundData1);
				sound->SoundPlayer(title);

			}
		}

		//デバック時のみEscキーで終了できるようにするここはあとで変更しても全然ダイジョブ
#ifdef _DEBUG
		if (Input::GetKeyDown(VK_ESCAPE)) break;
#endif
	}

	sound->SoundUnLoad(&soundData1);
	delete sound;
	delete debugCamer_;
	delete engine;

	return 0;
}