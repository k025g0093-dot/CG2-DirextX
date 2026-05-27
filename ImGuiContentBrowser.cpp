#include "ImGuiContentBrowser.h"
#include "TUFEngine.h"


static const std::filesystem::path s_AssetsPath = "resources";

ImGuiContentBrowser::ImGuiContentBrowser()
	:m_CurrentDirectory(s_AssetsPath)
{
}

//あとで変更予定

void ImGuiContentBrowser::update(TUFEngine* engine) {
#ifdef USE_IMGUI
	if (!show) return;

	if (begin("Content Browser")) {

		if (m_CurrentDirectory != std::filesystem::path(s_AssetsPath))
		{
			if (ImGui::Button("<-")) {
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}

		}
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetsPath);

			std::string filenameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
			{

				if (ImGui::Button(filenameString.c_str()))
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			else 
			{
				if (ImGui::Button(filenameString.c_str()))
				{

				}
			}
		}





		end();
	}
#endif
}