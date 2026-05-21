#pragma once

#include "ImGuiWindow.h"
#include <vector>
#include <memory> 

class TUFEngine;

class ImGuiUIManager {
public:
    ImGuiUIManager(HWND hwnd);
    ~ImGuiUIManager();
    void update(TUFEngine* engine);
    void render();

    void addWindow(std::shared_ptr<ImGuiUIWindow> newWin);

private:
    std::vector<std::shared_ptr<ImGuiUIWindow>> windows;

    void ShowDockSpace(bool* p_open);
    void setstyle();
    void updateWindows(TUFEngine* engine);
    void deleteWindows();
};