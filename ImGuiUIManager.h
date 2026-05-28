#pragma once

#include "ImGuiWindow.h"
#include <vector>
#include <memory> 
#include <functional>

class TUFEngine;

class ImGuiUIManager {
public:
    ImGuiUIManager(HWND hwnd);
    ~ImGuiUIManager();
    void update(TUFEngine* engine);
    void render();

    void addWindow(std::shared_ptr<ImGuiUIWindow> newWin);
    std::function<void()> onDrawGUI = nullptr;
    std::function<void(const std::wstring&)> onFileDrop = nullptr;
private:
    std::vector<std::shared_ptr<ImGuiUIWindow>> windows;

    void ShowDockSpace(bool* p_open);
    void setstyle();
    void updateWindows(TUFEngine* engine);
    void deleteWindows();
};