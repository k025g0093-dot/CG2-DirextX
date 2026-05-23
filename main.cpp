#include "TUFEngine.h"
#include "Sphere.h"
#include "WaveGrid.h"
#include "Camera.h"
#include <algorithm>

//保存用のファイル
#include <fstream>
#include <iomanip>



#include "Sound.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    SetUnhandledExceptionFilter(ExportDump);

    const int32_t kClineWidth = 1280;
    const int32_t kClineHeight = 720;

    TUFEngine* engine = new TUFEngine(kClineWidth, kClineHeight, L"TUFEngine");
    assert(engine->GetDevice() != nullptr);

    ShowWindow(engine->GetHwnd(), nCmdShow);


    int uvChecker = engine->LoadTexture("resources/uvChecker.png");
    int monsterBall = engine->LoadTexture("resources/monsterBall.png");
    int umi = engine->LoadTexture("resources/ao.jpg");

    MeshModel* modelData = engine->LoadModel("resources", "plane.obj");

    Sound* sound=new Sound;

    SoundData soundData1 = sound->SoundLoadWave("resources/fanfare.wav");

    HRESULT hr = S_OK;
    uint32_t sphereVertexCount = 16 * 16 * 6;



#pragma region WaveGrid

    const int cubeCountX = 100;
    const int cubeCountZ = 100;

    WaveGrid waveGrid(cubeCountX, cubeCountZ);

    int wallX = cubeCountX / 3;
    int holeStart = cubeCountZ / 2 - 3;
    int holeEnd = cubeCountZ / 2 + 3;
    for (int gz = 0; gz < cubeCountZ; gz++) {
        waveGrid.setWall(wallX, gz, (gz < holeStart || gz >= holeEnd));
    }

    float waveStrength = 10.0f;
    DynamicMesh mesh(cubeCountX, cubeCountZ);
    std::vector<Vector4> normalColors(cubeCountX * cubeCountZ);
    float t = 0.0f;

#pragma endregion
    engine->m_camera.transform.translate.x = -20.0f;

    engine->m_camera.transform.translate.y = 200.0f;
    engine->m_camera.transform.translate.z = -300.0f;
    engine->m_camera.transform.rotate.x = 0.6f;
    bool  useMonsterBall = true;
    float cameraRotateSpeed = 0.01f;
    float rotX = 0.01f;


    std::ofstream waveLog("wave_analysis.csv");
    waveLog << "frame,z,height,intensity\n";

    int frameIndex = 0;
    int observeX = wallX + 40;

    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else {
            engine->OnUpdate();

            // カメラ入力
            engine->m_camera.transform.rotate.y += Input::GetRightStickX() * cameraRotateSpeed;
            engine->m_camera.transform.rotate.x -= Input::GetRightStickY() * cameraRotateSpeed;
            engine->m_camera.transform.translate.x += Input::GetLeftStickX();
            engine->m_camera.transform.translate.z += Input::GetLeftStickY();

            // SetViewProjectionMatrix
            engine->SetViewProjectionMatrix(
                engine->m_camera.GetViewProjectionMatrix(kClineWidth, kClineHeight));



            // WaveGrid更新
            t += 0.016f;
            for (int gz = 1; gz < cubeCountZ - 1; gz++) {
                waveGrid.mCurrent[waveGrid.valueIndex(1, gz)] = sinf(t * 20.0f) * waveStrength;
            }
            waveGrid.update();

            if (frameIndex % 10 == 0) {
                for (int z = 0; z < cubeCountZ; z++) {
                    float h = waveGrid.getHeight(observeX, z);
                    float intensity = h * h;

                    waveLog << frameIndex << "," << z << "," << h << "," << intensity << "\n";
                }
            }
            frameIndex++;


            //---------------
            //描画更新処理
            //---------------

            engine->PreDraw();

            for (int i = 0; i < 2; ++i) {
                engine->DrawSphere(
                    { 0.0f + i * 5.0f, 0.0f, 5.0f },
                    { 0.0f, rotX, 0.0f },
                    { 1.0f, 1.0f, 1.0f },
                    useMonsterBall ? monsterBall : uvChecker);
            }

            engine->DrawMesh(modelData, { 0.0f,0.0f,0.0f }, { 0.0f,rotX,0.0f }, { 1.0f,1.0f,1.0f });

            for (int i = 0; i < 10; i++) {
                engine->DrawTriangle(
                    { 0.0f + i * 0.5f, 2.0f, 0.0f },
                    { 0.0f, rotX, 0.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1,1,1,1 });
            }

            //engine->DrawSprite(
            //    { 0.0f, 0.0f },
            //    360,360,
            //    { 0,0,0 },
            //    { 1,1,1 },
            //    { 1,1,1,1 },
            //    uvChecker);


            for (int iz = 0; iz < cubeCountZ; iz++) {
                for (int ix = 0; ix < cubeCountX; ix++) {
                    float h = waveGrid.getHeight(ix, iz);
                    mesh.updateHeight(ix, iz, h);
                    auto n = waveGrid.getNormal(ix, iz);
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
            engine->DrawDynamicMeshWithNormal(mesh, normalColors, umi);

            engine->PostDraw();


            if (Input::GetKeyDown(VK_SPACE)) {
                sound->SoundPlayWave(soundData1);
            }
        }

#ifdef _DEBUG
        if (Input::GetKeyDown(VK_ESCAPE)) break;
#endif
    }

#ifdef USE_IMGUI
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif
    delete sound;
    sound->SoundUnLoad(&soundData1);
    delete engine;

    return 0;
}
