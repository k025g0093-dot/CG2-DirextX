#pragma once
#include "allVector.h" // Vector3やMatrix4x4があるヘッダ

class Camera {
public:
    Camera() {
        eye = { 0.0f, 5.0f, -10.0f };
        target = { 0.0f, 0.0f,   0.0f };
        up = { 0.0f, 1.0f,   0.0f };
    }

    Matrix4x4 GetViewMatrix();
    Matrix4x4 GetProjMatrix(float width, float height);
    Matrix4x4 GetViewProjectionMatrix(float width, float height);

    Vector3 eye;
    Vector3 target;
    Vector3 up;
};