#include "ImGuiSpriteUV.h"
#include "TUFEngine.h"

void ImGuiSpriteUV::update(TUFEngine* engine) {
#ifdef USE_IMGUI
    if (!show) return;

    if (begin("Sprite UV")) {
        Vector2 uvScale = engine->GetSpriteUVScale();
        Vector2 uvTranslate = engine->GetSpriteUVTranslate();
        float uvRotate = engine->GetSpriteUVRotate();

        bool changed = false;
        changed |= ImGui::DragFloat2("UV Scale", &uvScale.x, 0.01f, -10.0f, 10.0f);
        changed |= ImGui::DragFloat("UV Rotate", &uvRotate, 0.01f, -6.28318f, 6.28318f);
        changed |= ImGui::DragFloat2("UV Translate", &uvTranslate.x, 0.01f, -10.0f, 10.0f);

        if (changed) {
            engine->SetSpriteUVScale(uvScale);
            engine->SetSpriteUVRotate(uvRotate);
            engine->SetSpriteUVTranslate(uvTranslate);
        }

        if (ImGui::Button("Reset UV")) {
            engine->ResetSpriteUVTransform();
        }

        end();
    }
#endif
}
