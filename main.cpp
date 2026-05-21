#include "TUFEngine.h"
#include "Sphere.h"
#include "WaveGrid.h"
#include "Camera.h"
#include <algorithm>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    SetUnhandledExceptionFilter(ExportDump);

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
    ShowWindow(engine->GetHwnd(), nCmdShow);

    int uvChecker = textureManager->LoadTexture("resources/uvChecker.png");
    int monsterBall = textureManager->LoadTexture("resources/monsterBall.png");

    MeshModel* modelData = engine->LoadModel("resources", "plane.obj");

    HRESULT hr = S_OK;
    uint32_t sphereVertexCount = 16 * 16 * 6;

#pragma region リソースたち

    ID3D12Resource* vertexResource = CreateVertexResource(
        engine->GetDevice(), sizeof(VertexData) * sphereVertexCount, hr);
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = CreateVertexBufferView(
        vertexResource, sizeof(VertexData) * sphereVertexCount, sizeof(VertexData));

    ID3D12Resource* vertexResourceSprite = CreateBufferResource(
        engine->GetDevice(), sizeof(VertexData) * sphereVertexCount);
    ID3D12Resource* indexResourceSprite = CreateBufferResource(
        engine->GetDevice(), sizeof(uint32_t) * 6);

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
    vertexDataSprite[0] = { {0.0f, 360.0f, 0.0f, 1.0f}, {0.0f, 1.0f} };
    vertexDataSprite[0].normal = { 0.0f, -1.0f, 0.0f };
    vertexDataSprite[1] = { {0.0f,   0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} };
    vertexDataSprite[2] = { {640.0f, 360.0f, 0.0f, 1.0f}, {1.0f, 1.0f} };
    vertexDataSprite[3] = { {640.0f,   0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} };
    vertexResourceSprite->Unmap(0, nullptr);

    uint32_t* indexDataSpraite = nullptr;
    indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSpraite));
    indexDataSpraite[0] = 0; indexDataSpraite[1] = 1; indexDataSpraite[2] = 2;
    indexDataSpraite[3] = 1; indexDataSpraite[4] = 3; indexDataSpraite[5] = 2;

    ID3D12Resource* materialResource = CreateBufferResource(engine->GetDevice(), sizeof(Material));
    Material* materialData = nullptr;
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->uvTransform = MakeIdentity4x4();
    materialData->enableLifhting = 0;

    ID3D12Resource* materialResourceSprite = CreateBufferResource(engine->GetDevice(), sizeof(Material));
    Material* materialDataSprite = nullptr;
    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
    materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialDataSprite->enableLifhting = false;
    materialDataSprite->uvTransform = MakeIdentity4x4();

    TransformData uvTransformSprite{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

    TransformData transformData{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

    ID3D12Resource* wvpResource = CreateBufferResource(engine->GetDevice(), sizeof(TransformationMatrix));
    TransformationMatrix* wvpData = nullptr;
    wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
    wvpData->WVP = MakeIdentity4x4();
    wvpData->World = MakeIdentity4x4();

    ID3D12Resource* transformationMatrixResourceSprite =
        CreateBufferResource(engine->GetDevice(), sizeof(TransformationMatrix));
    Matrix4x4* transformationMatrixDataSprite = nullptr;
    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
    *transformationMatrixDataSprite = MakeIdentity4x4();
    TransformData transformDataSprite{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

    ID3D12Resource* directionalLightDataResource =
        CreateBufferResource(engine->GetDevice(), sizeof(DirectionalLLight));
    DirectionalLLight* directionalLightData = nullptr;
    directionalLightDataResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
    directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
    directionalLightData->intensity = 1;
    engine->SetDirectionalLightResource(directionalLightDataResource);

#pragma endregion

#pragma region WaveGrid

    const int cubeCountX = 10;
    const int cubeCountZ = 10;

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

    // ★ Cameraクラスを使う

    bool  useMonsterBall = true;
    float cameraRotateSpeed = 0.01f;
    float rotX = 0.01f;

    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        
        }
        else {

            engine->OnUpdate();

            //カメラ入力
            engine->m_camera.transform.rotate.y += Input::GetRightStickX() * cameraRotateSpeed;
            engine->m_camera.transform.rotate.x -= Input::GetRightStickY() * cameraRotateSpeed;
            engine->m_camera.transform.translate.x += Input::GetLeftStickX();
            engine->m_camera.transform.translate.z += Input::GetLeftStickY();

            // SetViewProjectionMatrix
            engine->SetViewProjectionMatrix(
                engine->m_camera.GetViewProjectionMatrix(kClineWidth, kClineHeight));
            // オブジェクトのワールド行列（これはmainに残す）
            transformData.rotate.y += 0.01f;
            Matrix4x4 worldMatrix = MakeAffineMatrix(
                transformData.scale, transformData.rotate, transformData.translate);
            wvpData->WVP = Multiply(worldMatrix, engine->m_camera.GetViewProjectionMatrix(kClineWidth, kClineHeight));
            wvpData->World = worldMatrix;

            // スプライト用
            Matrix4x4 worldMatrixSprite = MakeAffineMatrix(
                transformDataSprite.scale, transformDataSprite.rotate, transformDataSprite.translate);
            Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(
                0.0f, 0.0f, (float)kClineWidth, (float)kClineHeight, 0.1f, 100.0f);
            *transformationMatrixDataSprite = Multiply(worldMatrixSprite, projectionMatrixSprite);

            Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
            uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
            uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
            materialDataSprite->uvTransform = uvTransformMatrix;

            // WaveGrid更新
            t += 0.016f;
            for (int gz = 1; gz < cubeCountZ - 1; gz++) {
                waveGrid.mCurrent[waveGrid.valueIndex(1, gz)] = sinf(t * 3.0f) * waveStrength;
            }
            waveGrid.update();

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

            engine->GetCommandList()->SetGraphicsRootSignature(engine->GetRootSignature());
            engine->GetCommandList()->SetPipelineState(engine->GetPipelineState());
            engine->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
            engine->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
                    int idx = iz * cubeCountX + ix;
                    normalColors[idx] = {
                        (n.x + 1.0f) / 2.0f,
                        (n.y + 1.0f) / 2.0f,
                        (n.z + 1.0f) / 2.0f,
                        1.0f
                    };
                }
            }
            engine->DrawDynamicMeshWithNormal(mesh, normalColors);

            engine->PostDraw();
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

    vertexResource->Release();
    materialResource->Release();
    delete engine;

    return 0;
}