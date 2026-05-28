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

	if (m_DirectoryIcon == -1) {
		m_DirectoryIcon = TextureManager::GetInstance()->LoadTexture("ContentBrowserAsset/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = TextureManager::GetInstance()->LoadTexture("ContentBrowserAsset/Icons/ContentBrowser/FileIcon.png");
	}

	if (!show) return;

	if (begin("Content Browser")) {

		if (m_CurrentDirectory != std::filesystem::path(s_AssetsPath))
		{
			if (ImGui::Button("<-")) {
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}

		}

		static float padding = 16.0f;
		static float thumbnailSize = 60;
		float cellSize = thumbnailSize + padding;

		float panelWindth = ImGui::GetContentRegionAvail().x;
		int colummCount = (int)(panelWindth / cellSize);
		if (colummCount < 1)
			colummCount = 1;
		ImGui::Columns(colummCount, 0, false);


		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetsPath);
			std::string filenameString = relativePath.filename().string();

			auto gpuHandle = TextureManager::GetInstance()->GetGPUHandle(directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon);

			ImGui::ImageButton(filenameString.c_str(), (ImTextureID)gpuHandle.ptr, { thumbnailSize,thumbnailSize });

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
			}

			ImGui::Text(filenameString.c_str());

			ImGui::NextColumn();

		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		end();
	}
#endif
}