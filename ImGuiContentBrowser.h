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

    std::filesystem::path currentPath_; // 現在表示しているフォルダ
    std::filesystem::path rootPath_;    // ルート（resources）に戻れるように保持

    int m_DirectoryIcon = -1;
    int m_FileIcon = -1;
};