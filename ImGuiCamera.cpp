#include "ImGuiCamera.h"
#include "TUFEngine.h"

void ImGuiCamera::update(TUFEngine* engine) {
#ifdef USE_IMGUI
    if (!show || !m_transform) return;

    if (begin("Camera")) {
        ImGui::DragFloat3("Camera Position", &m_transform->translate.x, 0.1f);
        ImGui::DragFloat3("Camera Rotation", &m_transform->rotate.x, 0.01f);

        Camera* camera = new Camera();
        if (camera) {
            float nearPlane = camera->GetNearPlane();
            float farPlane = camera->GetFarPlane();

            ImGui::Separator();
            ImGui::Text("Projection Settings");

            if (ImGui::SliderFloat("Near Plane", &nearPlane, 0.01f, 10.0f, "%.3f")) {
                camera->SetNearPlane(nearPlane);
            }
            if (ImGui::SliderFloat("Far Plane", &farPlane, 100.0f, 5000.0f, "%.1f")) {
                camera->SetFarPlane(farPlane);
            }

            // Far/Near の比率を表示（深度精度の目安）
            float ratio = farPlane / nearPlane;
            ImGui::Text("Ratio (Far/Near): %.0f", ratio);
            if (ratio > 10000.0f) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[⚠ Low precision]");
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Reset Camera")) {
            m_transform->translate = { 0.0f, 0.0f, -5.0f };
            m_transform->rotate = { 0.0f, 0.0f,  0.0f };
        }

        end();
    }
#endif
}