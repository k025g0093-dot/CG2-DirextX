#include "ImGuiWindow.h"
#include "TUFEngine.h"
#include "ModelManager.h"

ImGuiUIWindow::ImGuiUIWindow() : show(true) {}
ImGuiUIWindow::~ImGuiUIWindow() {}
#ifdef USE_IMGUI
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
#endif // DEBUG



//ここら辺は基底クラスと各種構造を理解するための仮実装です。今後、必要に応じて引数を追加したり、内容を充実させていきます。
// 💡 引数を追加
void ImGuiUIWindow::update(TUFEngine* engine) {
	if (begin("Default Window")) {
	}
	end();

}

void ImGuiUIWindow::Show() {
	show = true;
}

// ImGuiWindow.cpp
bool ImGuiUIWindow::begin(std::string name, ImGuiWindowFlags flags)
{
#ifdef USE_IMGUI
	if (!show) return false;
	return ImGui::Begin(name.c_str(), &show, flags);
#else
	return false;
#endif
}



void ImGuiUIWindow::end()
{
#ifdef USE_IMGUI
	ImGui::End();
#endif
}

//=============================
//IGStartupWindowの実装
//=============================



//ここも似た感じの者です将来的にはもっといろんな情報であったりほかのクラスと連動していろんなことができるようにしていきたいですね
void ImGuiSceneWindow::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	if (begin("シーン", ImGuiWindowFlags_MenuBar)) {

		// 🌟 メニューバー
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("ファイル")) {
				if (ImGui::MenuItem("保存")) {
					ModelManager::GetInstance()->SaveToFile();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// オブジェクト一覧
		auto& objects = EntityManager::GetInstance()->GetEntities();
		for (int i = 0; i < (int)objects.size(); i++) {
			ImGui::PushID(i);

			// オブジェクト名（パスからファイル名のみ抽出）
			std::string objName = objects[i]->name;
			size_t sep = objName.find_last_of("/\\");
			if (sep != std::string::npos) objName = objName.substr(sep + 1);
			ImGui::Text("%s", objName.c_str());
			ImGui::SameLine();

			// 選択ボタン
			if (ImGui::Button("選択")) {
				for (int j = 0; j < (int)objects.size(); j++) {
					objects[j]->isSelected = false;
				}
				objects[i]->isSelected = true;
			}

			// 選択状態のバッジ表示
			if (objects[i]->isSelected) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[選択中]");
			}

			ImGui::Dummy(ImVec2(0.0f, 6.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 6.0f));

			ImGui::PopID();
		}

	}
	end();

#endif
}


//ギズモの仮実装。まだまだいらないものとか将来的に自由にアイテムを選択できるようにしたりしていきたい
void ImGuiZmoWindow::update(TUFEngine* engine) {
#ifdef _DEBUG
	auto& objects =EntityManager::GetInstance()->GetEntities();
	if (objects.empty()) return; // オブジェクトがなければ何もしない

	// キー入力による切り替え判定だけを行う
	if (ImGui::IsKeyPressed(ImGuiKey_T)) mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) mCurrentGizmoOperation = ImGuizmo::SCALE;
#endif
}




// --- ImGuiWindow.cpp の最後に追記 ---

void ImGuiViewportWindow::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGuizmo::IsOver()) {
		viewportFlags |= ImGuiWindowFlags_NoMove;
	}

	if (ImGui::Begin("デバック用ビューポート", nullptr, viewportFlags)) {

		

		// --- ① シーン（ゲーム画面）の描画 ---
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		engine->SetCurrentRenderSize(viewportSize.x, viewportSize.y);

		// 🌟重要：タイトルバーを排除した、画像が描画される正確なスクリーン左上座標を取得
		ImVec2 imageScreenPos = ImGui::GetCursorScreenPos();

		ImTextureID sceneTextureId = (ImTextureID)engine->GetSceneSrvGpuHandle().ptr;

		ImGui::Image(
			sceneTextureId,
			viewportSize,
			ImVec2(0.0f, 0.0f),
			ImVec2(1.0f, 1.0f),
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f), // Tint（画像の色：白＝そのまま）
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)  // 🌟Border（枠線の色：透明。ここが1.0fで黒枠になると1ピクセルズレる原因になります）
		);

		// --- ② ギズモ（ImGuizmo）の描画 ---
#ifdef _DEBUG
		LightManager* lm = LightManager::GetInstance();
		int selectedLight = lm->GetSelectedLight();

		auto& objects = EntityManager::GetInstance()->GetEntities();

		bool hasEntitySelection = false;
		for (auto& obj : objects) {
			if (obj->isSelected) { hasEntitySelection = true; break; }
		}

		if (selectedLight >= 1 && selectedLight < LightManager::MAX_LIGHTS) {
			// 🌟ライトのギズモ操作
			ImGuizmo::BeginFrame();
			ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
			ImGuizmo::SetRect(imageScreenPos.x, imageScreenPos.y, viewportSize.x, viewportSize.y);

			LightData light = lm->GetLight(selectedLight);

			// 位置だけを持つアフィン行列を作る（回転・スケールは単位のまま）
			Matrix4x4 lightTransform = MakeAffineMatrix(
				{ 1.0f, 1.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f },
				light.dirOrPos
			);

			if (ImGuizmo::Manipulate(
				&engine->GetViewMatrix().m[0][0],
				&engine->GetProjectionMatrix().m[0][0],
				ImGuizmo::TRANSLATE, // ライトは移動だけできれば十分
				ImGuizmo::LOCAL,
				&lightTransform.m[0][0]
			)) {
				float t[3], r[3], s[3];
				ImGuizmo::DecomposeMatrixToComponents(&lightTransform.m[0][0], t, r, s);
				light.dirOrPos = { t[0], t[1], t[2] };
				lm->SetLight(selectedLight, light);
			}
		}
		else if (hasEntitySelection) {
			// 🌟既存のEntity用ギズモ処理
			ImGuizmo::BeginFrame();
			ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

			// 領域の設定（これは imageScreenPos で正しい）
			ImGuizmo::SetRect(imageScreenPos.x, imageScreenPos.y, viewportSize.x, viewportSize.y);

			constexpr float kDegToRad = 3.1415926535f / 180.0f;

			for (int i = 0; i < (int)objects.size(); i++) {
				if (!objects[i]->isSelected) continue;

				Matrix4x4 transform = MakeAffineMatrix(objects[i]->transform.scale, objects[i]->transform.rotation, objects[i]->transform.position);

				if (ImGuizmo::Manipulate(
					&engine->GetViewMatrix().m[0][0],
					&engine->GetProjectionMatrix().m[0][0],
					mCurrentGizmoOperation,
					mCurrentGizmoMode,
					&transform.m[0][0]
				)) {
					float matrixTranslation[3], matrixRotation[3], matrixScale[3];
					ImGuizmo::DecomposeMatrixToComponents(&transform.m[0][0], matrixTranslation, matrixRotation, matrixScale);

					objects[i]->transform.position = { matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] };
					objects[i]->transform.rotation = { matrixRotation[0] * kDegToRad, matrixRotation[1] * kDegToRad, matrixRotation[2] * kDegToRad };
					objects[i]->transform.scale = { matrixScale[0], matrixScale[1], matrixScale[2] };
				}
			}
		}
#endif // _DEBUG

		// --- ③ ドロップターゲットの設定 ---
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* itemPath = (const wchar_t*)payload->Data;
				auto manager = engine->GetImGuiManager();
				if (manager && manager->onFileDrop) {
					manager->onFileDrop(itemPath);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
#endif
}






void ImGuiPlayViewportWindow::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	if (ImGui::Begin("プレイビューポート", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse)) {

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		ImVec2 imageScreenPos = ImGui::GetCursorScreenPos();

		ImTextureID sceneTextureId = (ImTextureID)engine->GetSceneSrvGpuHandle().ptr;

		ImGui::Image(
			sceneTextureId,
			viewportSize,
			ImVec2(0.0f, 0.0f),
			ImVec2(1.0f, 1.0f),
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		);
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
#endif
}



void ImGuiComponentWindow::update(TUFEngine* engine)
{
#ifdef USE_IMGUI


	// 🌟全体のパディング（内側の余白）を少し広げる
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));


	if (begin("コンポーネント")){

		


		if (ImGui::CollapsingHeader("ライト設定")) {
			LightManager* lm = LightManager::GetInstance();

			ImGui::Dummy(ImVec2(0.0f, 2.0f));

			if (ImGui::CollapsingHeader("グローバルライト")) {
				LightData global = lm->GetLight(0);
				bool changed = false;
				changed |= ImGui::ColorEdit4("色", &global.color.x);
				changed |= ImGui::DragFloat3("方向", &global.dirOrPos.x, 0.01f, -1.0f, 1.0f);
				changed |= ImGui::DragFloat("強度", &global.intensity, 0.01f, 0.0f, 10.0f);
				if (changed) lm->SetLight(0, global);
			}

			ImGui::Dummy(ImVec2(0.0f, 4.0f));

			// ポイントライト（index 1〜MAX_LIGHTS-1）
			// ポイントライト（index 1〜MAX_LIGHTS-1）
			for (int i = 1; i < LightManager::MAX_LIGHTS; i++) {
				ImGui::PushID(i);
				std::string label = "ポイントライト " + std::to_string(i);
				if (ImGui::CollapsingHeader(label.c_str())) {
					LightData point = lm->GetLight(i);
					bool changed = false;
					changed |= ImGui::ColorEdit4("色", &point.color.x);
					changed |= ImGui::DragFloat3("位置", &point.dirOrPos.x, 0.1f);
					changed |= ImGui::DragFloat("強度", &point.intensity, 0.01f, 0.0f, 10.0f);

					// 🌟追加：ギズモ操作対象として選択
					if (ImGui::Button("ギズモで選択")) {
						lm->SetSelectedLight(i);
					}

					if (changed) {
						point.type = 1;
						lm->SetLight(i, point);
					}
				}
				ImGui::PopID();
			}
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 10.0f));



		for (auto& entity : EntityManager::GetInstance()->GetEntities()) {
			if (!entity->isSelected) continue;

			if (ImGui::CollapsingHeader("トランスフォーム")) {
				ImGui::DragFloat3("位置", &entity->transform.position.x, 0.1f);
				ImGui::DragFloat3("回転", &entity->transform.rotation.x, 0.01f);
				ImGui::DragFloat3("拡大", &entity->transform.scale.x, 0.01f, 0.01f, 10.0f);
			}

			for (auto* c : entity->GetComponents()) {
				const char* typeName = typeid(*c).name();
				if (ImGui::CollapsingHeader(typeName)) {
					if (auto* mf = dynamic_cast<MeshFilter*>(c)) {
					
						ImGui::Spacing();
						ImGui::Text("モデル: %s", mf->model ? "読み込み済み" : "なし");
						ImGui::Spacing();

					}
					else if (auto* lc = dynamic_cast<LearnComponent*>(c)) {
						ImGui::Spacing();
						ImGui::Text("ステータス: 実行中");
					
						ImGui::Spacing();

					}
					else if (auto* gs = dynamic_cast<GameScript*>(c)) {

						ImGui::Text("ここはC#のスクリプトの作成から実行までを行っています");

						ImGui::Spacing();

						
						ImGui::Text("これはからスクリプトの初期化またヴィジュアルスタジオを起動します");
						if (ImGui::Button("C#スクリプトを初期化＆起動")) {

							gs->StartScript(); // ボタンを押した時に1回だけ初期化されるので安全！
						}

						ImGui::Spacing();

						ImGui::Text("これは実際に書いたスクリプトののビルドを行います");
						if (ImGui::Button("C#のビルド")) {
							gs->ReloadScript();
							gs->Update();
							
						}			

						
						ImGui::Text("現在の.csのクラス名を決めるところです");

						ImGui::Text("test: %s", gs->m_scriptNameBuf);
						if (ImGui::InputText("作成するスクリプトの名前（.csクラス）##script",
							gs->m_scriptNameBuf, sizeof(gs->m_scriptNameBuf))) {
							gs->m_scriptName = gs->m_scriptNameBuf;
						}

						if (ImGui::Button("スクリプト作成")) {
							//ここで新しいスクリプトと同時にヴィジュアルスタジオを立ち上げる
							gs->StartScript();
						}

					}
				}
			}

			ImGui::Separator();
			if (ImGui::Button("+ コンポーネント追加")) {
				ImGui::OpenPopup("AddComponentPopup");
			}
			if (ImGui::BeginPopup("AddComponentPopup")) {
				if (!entity->GetComponent<MeshFilter>() && ImGui::MenuItem("MeshFilter"))
					entity->AddComponent<MeshFilter>();
				if (!entity->GetComponent<LearnComponent>() && ImGui::MenuItem("LearnComponent"))
					entity->AddComponent<LearnComponent>();

				if (!entity->GetComponent<GameScript>() && ImGui::MenuItem("C#スクリプト"))
					entity->AddComponent<GameScript>();

				ImGui::EndPopup();
			}

			ImGui::Spacing();
			if (ImGui::Button("削除")) {
				EntityManager::GetInstance()->DestroyEntity(entity.get());
				ModelManager::GetInstance()->SaveToFile();
				break;
			}
		}
	}


	ImGui::PopStyleVar(); // スタイル設定を元に戻す


	ImGui::End();

#endif // USE_IMGUI

}
