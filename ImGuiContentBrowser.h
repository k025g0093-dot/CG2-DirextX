#pragma once
#include "ImGuiWindow.h"
#include <filesystem>
#include <string>

class ImGuiContentBrowser : public ImGuiUIWindow {
public:
    ImGuiContentBrowser();
    void update(TUFEngine* engine) override;
private:

    std::filesystem::path m_CurrentDirectory;
    int m_DirectoryIcon = -1;
    int m_FileIcon = -1;
};