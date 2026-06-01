#pragma once
#include "ConvertString.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#include "externals/imgui/imguizmo.h"

#endif // USE_IMGUI

// 🌟 超重要：相互参照バグを防ぐための前方宣言
class TUFEngine;

class ImGuiUIWindow
{
public:
    ImGuiUIWindow();
    ~ImGuiUIWindow();

    // 💡 引数に TUFEngine* を追加（virtualなので子クラスもこれに合わせる）
    virtual void update(TUFEngine* engine);
    void Show();

protected:
    virtual bool begin(std::string name);
    void end();
    bool show = true; // 💡 最初から表示状態（true）にしておくとバグりにくいです
};

class IGStartupWindow : public ImGuiUIWindow
{
public:
    // 💡 親クラスに合わせて引数を追加
    void update(TUFEngine* engine) override;
private:
    int counter = 0;
};

// ImGuiWindow.h に追加
class ImGuiSceneWindow : public ImGuiUIWindow
{
public:
    void update(TUFEngine* engine) override;
};

class ImGuiViewportWindow : public ImGuiUIWindow
{
public:
    void update(TUFEngine* engine) override;
};

class ImGuiZmoWindow : public ImGuiUIWindow
{
public:
    int selectedIndex = -1;
	void update(TUFEngine* engine) override;
};
