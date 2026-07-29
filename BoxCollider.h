#pragma once
#include"Collider.h"
#include "allVector.h"
class BoxCollider :public Collider {
public:
    Vector3 size = { 1.0f, 1.0f, 1.0f };

    const char* GetShapeName() const override { return "BoxCollider"; }
    Component* Clone() const override {
        auto* c = new BoxCollider();
        c->size = size;
        c->isTrigger = isTrigger;
        return c;
    }
};
