#pragma once
#include "Vector.h"

class Camera {
public:
    Camera() {
        transform.scale     = { 1.0f, 1.0f,  1.0f };
        transform.rotate    = { 0.0f, 0.0f,  0.0f };
        transform.translate = { 0.0f, 0.0f, -10.0f };
    }

    Matrix4x4 GetViewMatrix() {
        Matrix4x4 cameraMatrix = MakeAffineMatrix(
            transform.scale, transform.rotate, transform.translate);
        return Inverse(cameraMatrix);
    }

    Matrix4x4 GetProjectionMatrix(float width, float height) {
        return MakePerspectiveFovMatrix(0.45f, width / height, 0.1f, 100.0f);
    }

    Matrix4x4 GetViewProjectionMatrix(float width, float height) {
        Matrix4x4 cameraMatrix = MakeAffineMatrix(
            transform.scale, transform.rotate, transform.translate);
        Matrix4x4 viewMatrix = Inverse(cameraMatrix);
        Matrix4x4 projMatrix = MakePerspectiveFovMatrix(
            0.45f, width / height, 0.1f, 100.0f);
        return Multiply(viewMatrix, projMatrix);
    }

    TransformData transform;
};