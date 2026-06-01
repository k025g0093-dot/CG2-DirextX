#include "ImGuiWindow.h"
#include "TUFEngine.h" // 🌟 ここで include することで、engineの中身が解禁される！

ImGuiUIWindow::ImGuiUIWindow() : show(true) {}
ImGuiUIWindow::~ImGuiUIWindow() {}

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

// 💡 引数を追加
void IGStartupWindow::update(TUFEngine* engine)
{
#ifdef USE_IMGUI
	if (show)
	{
		// beginが正常に開いたときだけ中身を描く（Endの重複呼び出しバグを防ぐ安全な書き方に微調整しました）
		if (begin("Startup"))
		{
			ImGui::Text("Press me:");
			if (ImGui::Button("button"))
			{

				counter++;
			}
			ImGui::Text("Counter: %d", counter);

			end(); // 💡 開いたので閉じる
		}
	}
#endif
}


// ImGuiWindow.cpp に追加
void ImGuiSceneWindow::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	if (begin("Scene")) {
		auto& objects = engine->GetDroppedMeshes();
		for (int i = 0; i < (int)objects.size(); i++) {
			ImGui::PushID(i);
			ImGui::Text(objects[i].name.c_str());
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


void ImGuiZmoWindow::update(TUFEngine* engine) {
#ifdef _DEBUG

	// "ImGuizmo Test" という名前の新しいウィンドウを作る
	if (begin("ImGuizmo Test")) {
	
		ImGuizmo::BeginFrame();          // フレームの開始を宣言
		ImGuizmo::SetOrthographic(false);

		float transform[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
		auto& objects = engine->GetDroppedMeshes();
		for (int i = 0; i < (int)objects.size(); i++) {
			objects[i].pos;
			objects[i].rot;
			objects[i].scale;
			transform;

			transform[0] = objects[i].scale.x * cosf(objects[i].rot.y) * cosf(objects[i].rot.x);
			transform[1] = objects[i].scale.x * sinf(objects[i].rot.x);
			transform[2] = objects[i].scale.x * sinf(objects[i].rot.y) * cosf(objects[i].rot.x);
			transform[3] = 0.0f;
			transform[4] = 0.0f;
			transform[5] = objects[i].scale.y * cosf(objects[i].rot.x);
			transform[6] = 0.0f;
			transform[7] = 0.0f;
			transform[8] = -objects[i].scale.z * sinf(objects[i].rot.y) * cosf(objects[i].rot.x);
			transform[9] = -objects[i].scale.z * sinf(objects[i].rot.x);
			transform[10] = objects[i].scale.z * cosf(objects[i].rot.y) * cosf(objects[i].rot.x);
			transform[11] = 0.0f;
			transform[12] = objects[i].pos.x;
			transform[13] = objects[i].pos.y;
			transform[14] = objects[i].pos.z;
			transform[15] = 1.0f;

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y); // 描画範囲
			ImGuizmo::Manipulate(
				&engine->GetViewMatrix().m[0][0],
				&engine->GetProjectionMatrix().m[0][0],
				ImGuizmo::TRANSLATE,
				ImGuizmo::LOCAL,
				transform
			);
		}
		end();
	}


#endif // _DEBUG
}

// --- ImGuiWindow.cpp の最後に追記 ---

void ImGuiViewportWindow::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	// "Game Viewport" という名前の新しいウィンドウを作る
	if (begin("dorp Obj")) {

		ImGui::Text("drop to obj");

		// ウィンドウの残り全域をドロップ可能なエリア（ダミー領域）にする
		ImVec2 availableSize = ImGui::GetContentRegionAvail();
		ImGui::Dummy(availableSize);

		// ★このウィンドウ（直前に作ったDummy領域）をドロップの受け皿にする
		if (ImGui::BeginDragDropTarget()) {
			// マウスが離された瞬間だけデータを取り出す
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* itemPath = (const wchar_t*)payload->Data;

				// TUFEngine の OnFileDropped を呼び出す
				auto manager = engine->GetImGuiManager();
				if (manager && manager->onFileDrop) {
					manager->onFileDrop(itemPath);
				}
			}
			ImGui::EndDragDropTarget();
		}

		end();
	}
#endif
}



