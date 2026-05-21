#include "ImGuiUIManager.h"
#include "TUFEngine.h" // 💡 ここで初めてインクルードする

// コンストラクタやデストラクタ、addWindowの実装例
ImGuiUIManager::ImGuiUIManager(HWND hwnd)
{
#ifdef USE_IMGUI
    // 💡 1. ImGuiのコンテキスト（本体）を作成
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 💡 2. ディレトクリやレイアウトを保存する設定の読み込み（基本はおまかせでOK）
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // 🌟 3. 【超重要】ドッキングとマルチウィンドウの最強フラグをここで叩き込む！
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // キーボード操作を有効化（お好みで）
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // 👈 これでウィンドウ同士がくっつくようになる！
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // 👈 これでウィンドウを画面の外に切り離せるようになる！

    // 💡 4. カラーテーマをダークモードに設定
    ImGui::StyleColorsDark();

    // 💡 5. スタイルの微調整（マルチウィンドウ時に枠線が綺麗に見えるようにする調整）
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
#endif
}
ImGuiUIManager::~ImGuiUIManager() {}

void ImGuiUIManager::addWindow(std::shared_ptr<ImGuiUIWindow> newWin) {
    windows.push_back(newWin);
}

void ImGuiUIManager::updateWindows(TUFEngine* engine) {
    // 🌟 登録されたウィンドウたちに「ほい、エンジンのポインタだよ」と渡して回る
    for (auto& win : windows) {
        win->update(engine);
    }
}

// 💡 引数に engine を追加
void ImGuiUIManager::update(TUFEngine* engine)
{
#ifdef USE_IMGUI
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    // 🌟 ウィンドウたちの更新
    updateWindows(engine);

    ImGui::Render();

    // マルチウィンドウ用の処理
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();

        // 🌟【ここを nullptr から書き換えます！】
        // 引数で貰った engine からデバイスとコマンドリストを引っこ抜いて ImGui に渡す！
        ImGui::RenderPlatformWindowsDefault(
            nullptr, 
            (void*)engine->GetCommandList()
        );
    }
#endif
}

void ImGuiUIManager::render() {
    // 必要に応じて描画コマンドをここに書く（通常はメイン側で ImGui_ImplDX12_RenderDrawData を呼べばOK）
}