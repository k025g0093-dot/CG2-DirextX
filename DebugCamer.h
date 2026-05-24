#pragma once
#include "allVector.h"
#include "Input.h"

class DebugCamer {
public:
    void Initialize(float width, float height);
    void Update();
    Matrix4x4 GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
    bool IsDebug() const { return isDebug; }
private:
    struct TransformData {
        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;
    };

    bool isDebug = false;

    float width_ = 0.0f;
    float height_ = 0.0f;

    TransformData transform_ = {
        { 1, 1,   1 },  // scale
        { 0, 0,   0 },  // rotate
        { 0, 0, -50 },  // translate
    };

    Matrix4x4 viewProjectionMatrix_{};
};