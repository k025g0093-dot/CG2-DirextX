#include "ImGuiUIManager.h"
#include "TUFEngine.h"

ImGuiUIManager::ImGuiUIManager(HWND hwnd)
{
#ifdef USE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

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
    for (auto& win : windows) {
        win->update(engine);
    }
}

void ImGuiUIManager::update(TUFEngine* engine)
{
#ifdef USE_IMGUI
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    updateWindows(engine);

    if (onDrawGUI) { onDrawGUI(); }

    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(
            nullptr,
            (void*)engine->GetCommandList()
        );
    }
#endif
}

void ImGuiUIManager::render() {}