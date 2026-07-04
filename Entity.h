#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Component.h"
#include "Transform.h"
#include "Create3DObjectOBB.h"



class Entity {
public:



    Entity() = default;
    ~Entity();

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        T* comp = new T(std::forward<Args>(args)...);
        m_components.push_back(comp);
        return comp;
    }

    template<typename T>
    T* GetComponent() {
        for (auto* c : m_components) {
            T* casted = dynamic_cast<T*>(c);
            if (casted) return casted;
        }
        return nullptr;
    }

    std::string name;
    Transform transform;

	bool isSelected = false;

    // Entity.h に追加
    OBB obb;
    AABB localAABB;

private:
    friend class EntityManager;
    std::vector<Component*> m_components;
};
