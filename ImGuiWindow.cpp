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