#pragma once
#include "Component.h"

class MeshModel;

class MeshFilter : public Component {
public:
    MeshModel* model = nullptr;
};
