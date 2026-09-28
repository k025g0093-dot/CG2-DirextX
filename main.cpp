#include "TUFEngine.h"
#include "ModelManager.h"
#include "Sphere.h"
#include "WaveGrid.h"
#include "Camera.h"
#include <algorithm>
#include <vector>
#include <cmath>

#include <fstream>
#include <iomanip>

#include "DebugCamer.h"
#include "Sound.h"

#include "LearnComponent.h"

#ifdef USE_IMGUI
#include "externals/imgui/imguizmo.h"
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	SetUnhandledExceptionFilter(ExportDump);



	const int32_t kClineWidth = 1280;
	const int32_t kClineHeight = 720;


	TUFEngine* engine = new TUFEngine(kClineWidth, kClineHeight, L"CG2_TUFEngine_LE2B_29_ヤマト_ユウヤ");
	assert(engine->GetDevice() != nullptr);

	ShowWindow(engine->GetHwnd(), nCmdShow);

	MeshModel* modelData = ModelManager::GetInstance()->LoadModel("resources/skyBox", "skyDome.fbx");
	if (modelData) modelData->SetEnableLighting(0);

	Sound* sound = new Sound;
	SoundData soundData1 = sound->SoundLoad("resources/fanfare.wav");
	SoundData title = sound->SoundLoad("resources/title.mp3");

	DebugCamer::GetInstance().Initialize((float)kClineWidth, (float)kClineHeight);

	const int cubeCountX = 200;
	const int cubeCountZ = 200;

	WaveGrid waveGrid(cubeCountX, cubeCountZ, ModelManager::GetInstance()->GetSceneObjects());

	// ========== GPU初期化 ==========
	waveGrid.InitializeGPU(engine->GetDevice(), engine);

	float waveStrength = 10.0f;

	engine->m_camera.transform.translate.x = 0.0f;
	engine->m_camera.transform.translate.y = 5.0f;
	engine->m_camera.transform.translate.z = -20.0f;
	engine->m_camera.transform.rotate.x = 0.0f;

	float cameraRotateSpeed = 0.016f;

	Vector3 meshPos = { 0.0f, 0.0f, 0.0f };
	Vector3 meshRot = { 0.0f, 0.0f, 0.0f };
	Vector3 meshScale = { 1.0f, 1.0f, 1.0f };

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

				if (ImGui::Begin("シーン設定")) {

					if (ImGui::CollapsingHeader("ウェーブ設定")) {
						ImGui::DragFloat("波形の強さ", &waveStrength, 0.1f, 0.0f, 50.0f);
					}

					if (ImGui::CollapsingHeader("メッシュ設定")) {
						ImGui::DragFloat3("位置", &meshPos.x, 0.1f);
						ImGui::DragFloat3("回転", &meshRot.x, 0.01f);
						ImGui::DragFloat3("拡大", &meshScale.x, 0.01f, 0.01f, 10.0f);
					}

				}
				ImGui::End();
				};
#endif // USE_IMGUI

			engine->m_camera.transform.rotate.y += Input::GetRightStickX() * cameraRotateSpeed;
			engine->m_camera.transform.rotate.x -= Input::GetRightStickY() * cameraRotateSpeed;
			engine->m_camera.transform.translate.x += Input::GetLeftStickX();
			engine->m_camera.transform.translate.z += Input::GetLeftStickY();

			DebugCamer::GetInstance().Update();

			if (DebugCamer::GetInstance().IsDebug()) {
				engine->SetViewProjectionMatrix(DebugCamer::GetInstance().GetViewProjectionMatrix());
			}
			else {
				engine->ResetViewProjectionMatrix();
			}



			////================================================================================================================
			////描画処理ここから
			////================================================================================================================

			engine->PreDraw();

			meshRot.y += 0.0016f;

			engine->DrawMesh(modelData, meshPos, meshRot, meshScale, -1);

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
	delete engine;

	return 0;
}