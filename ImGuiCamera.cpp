#include "ImGuiCamera.h"
#include "TUFEngine.h"

void ImGuiCamera::update(TUFEngine* engine) {
#ifdef USE_IMGUI
    if (!show || !m_transform) return;

    if (begin("Camera")) {
        ImGui::DragFloat3("Camera Position", &m_transform->translate.x, 0.1f);
        ImGui::DragFloat3("Camera Rotation", &m_transform->rotate.x, 0.01f);

        if (ImGui::Button("Reset Camera")) {
            m_transform->translate = { 0.0f, 0.0f, -5.0f };
            m_transform->rotate = { 0.0f, 0.0f,  0.0f };
        }

        end();
    }
#endif
}