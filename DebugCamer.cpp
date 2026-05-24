#include "DebugCamer.h"

void DebugCamer::Initialize(float width, float height) {
    width_ = width;
    height_ = height;
}

void DebugCamer::Update() {
    if (!isDebug) {
        if (Input::GetKeyDown('1')) {
            isDebug = true;
        }
    }
    else {
        if (Input::GetKeyDown('1')) {
            isDebug = false;
        }
    }

    if (isDebug) {
        if (Input::GetMouseButton(1)) {
            transform_.rotate.y += Input::GetMouseDeltaX() * 0.005f;
            transform_.rotate.x += Input::GetMouseDeltaY() * 0.005f;
        }

        if (Input::GetMouseButton(2)) {
            transform_.translate.x -= Input::GetMouseDeltaX() * 0.05f;
            transform_.translate.y += Input::GetMouseDeltaY() * 0.05f;
        }

        Matrix4x4 viewMatrix = Inverse(MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate));
        Matrix4x4 projMatrix = MakePerspectiveFovMatrix(0.45f, width_ / height_, 0.1f, 100.0f);
        viewProjectionMatrix_ = Multiply(viewMatrix, projMatrix);
    }
}