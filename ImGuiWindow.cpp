#include "ImGuiWindow.h"
#include "TUFEngine.h" // 🌟 ここで include することで、engineの中身が解禁される！

ImGuiUIWindow::ImGuiUIWindow() : show(true) {}
ImGuiUIWindow::~ImGuiUIWindow() {}


//ここら辺は基底クラスと各種構造を理解するための仮実装です。今後、必要に応じて引数を追加したり、内容を充実させていきます。
// 💡 引数を追加
void ImGuiUIWindow::update(TUFEngine* engine) {
	if (begin("Default Window")) {
		end();
	}
}

void ImGuiUIWindow::Show() {
	show = true;
}

bool ImGuiUIWindow::begin(std::string name)
{
#ifdef USE_IMGUI
	if (!show) return false;
	return ImGui::Begin(name.c_str(), &show);
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



//ここも似た感じの者です
void ImGuiSceneWindow::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	if (begin("Scene")) {
		auto& objects = engine->GetDroppedMeshes();

		for (int i = 0; i < (int)objects.size(); i++) {
			ImGui::PushID(i);

			ImGui::Text(objects[i].name.c_str());
			ImGui::SameLine(); // ボタンを横並びにする

			if (ImGui::Button("Select")) {
				for (int j = 0; j < (int)objects.size(); j++) {
					objects[j].isSelected = false;
				}
				objects[i].isSelected = true;
			}

			if (objects[i].isSelected) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[Selected]");
			}

			ImGui::DragFloat3("Position", &objects[i].pos.x, 0.1f);
			ImGui::DragFloat3("Rotation", &objects[i].rot.x, 0.01f);
			ImGui::DragFloat3("Scale", &objects[i].scale.x, 0.01f, 0.01f, 10.0f);

			if (ImGui::Button("Delete")) {
				engine->RemoveDroppedMesh(i);
				ImGui::PopID();
				break;
			}
			ImGui::Separator();
			ImGui::PopID();
		}
		end();
	}
#endif
}


//ギズモの仮実装。まだまだいらないものとか将来的に自由にアイテムを選択できるようにしたりしていきたい
void ImGuiZmoWindow::update(TUFEngine* engine) {
#ifdef _DEBUG

	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
	auto& objects = engine->GetDroppedMeshes();
	constexpr float kDegToRad = 3.1415926535f / 180.0f;

	// オブジェクトが一つもないときは、そもそもギズモを出す必要がないので、早期リターンする
	if (objects.empty()) {
		return;
	}

	if (selectedIndex < 0 || selectedIndex >= static_cast<int>(objects.size())) {
		selectedIndex = 0;
	}

	ImGuizmo::BeginFrame();
	ImGuizmo::SetOrthographic(false);

	if (ImGui::IsKeyPressed(ImGuiKey_T))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	for (int i = 0; i < (int)objects.size(); i++) {
		if (objects[i].isSelected == false) {
			continue;
		}

		Matrix4x4 transform = MakeAffineMatrix(
			objects[i].scale,
			objects[i].rot,
			objects[i].pos
		);

		float matrixTranslation[3], matrixRotation[3], matrixScale[3];

		if (begin("Local or World")) {


			if (mCurrentGizmoOperation != ImGuizmo::SCALE)
			{
				if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
					mCurrentGizmoMode = ImGuizmo::LOCAL;
				ImGui::SameLine();
				if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
					mCurrentGizmoMode = ImGuizmo::WORLD;
			}
			end();
		}
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		if (ImGuizmo::Manipulate(
			&engine->GetViewMatrix().m[0][0],
			&engine->GetProjectionMatrix().m[0][0],
			mCurrentGizmoOperation,
			mCurrentGizmoMode,
			&transform.m[0][0]
		)) {
			ImGuizmo::DecomposeMatrixToComponents(&transform.m[0][0], matrixTranslation, matrixRotation, matrixScale);

			objects[i].pos.x = matrixTranslation[0];
			objects[i].pos.y = matrixTranslation[1];
			objects[i].pos.z = matrixTranslation[2];

			objects[i].rot.x = matrixRotation[0] * kDegToRad;
			objects[i].rot.y = matrixRotation[1] * kDegToRad;
			objects[i].rot.z = matrixRotation[2] * kDegToRad;

			objects[i].scale.x = matrixScale[0];
			objects[i].scale.y = matrixScale[1];
			objects[i].scale.z = matrixScale[2];
		}
	}

#endif // _DEBUG
}




// --- ImGuiWindow.cpp の最後に追記 ---


//開発路駐できたら神なんだけど、とりあえずはここに ImGuiViewportWindow の実装を置いておきます。将来的には別ファイルに分けるかもしれません。
void ImGuiViewportWindow::update(TUFEngine* engine) {
//#ifdef USE_IMGUI
//	// "Game Viewport" という名前の新しいウィンドウを作る
//
//
//	ImGui::Text("Scene");
//	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
//	if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
//		engine->SetViewportSize(viewportPanelSize.x, viewportPanelSize.y);
//	}
//	ImGui::Dummy(viewportPanelSize); // ビューポートのサイズを確保するためのダミーアイテム
//	
//
//	// ウィンドウの残り全域をドロップ可能なエリア（ダミー領域）にする
//	if(ImGui::Begin("ViewportWindow")) {
//		ImVec2 availableSize = ImGui::GetContentRegionAvail();
//		ImGui::Dummy(availableSize);
//		if (ImGui::BeginDragDropTarget()) {
//
//			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
//				const wchar_t* itemPath = (const wchar_t*)payload->Data;
//
//				// TUFEngine の OnFileDropped を呼び出す
//				auto manager = engine->GetImGuiManager();
//				if (manager && manager->onFileDrop) {
//					manager->onFileDrop(itemPath);
//				}
//			}
//			ImGui::EndDragDropTarget();
//		}
//	}
//	ImGui::End();
//#endif
}



